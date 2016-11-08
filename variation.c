#include <math.h>
#include "firegpu.h"
#include "flame.h"
#include "xform.h"
#include "rng.h"
#include "log.h"
#include "source.h"
#include "variation.h"

#define PREVENT_DIVIDE_BY_ZERO	0.0000000001f

#define PRECALC_NONE		0
#define PRECALC_R_SQUARED	1
#define PRECALC_R			2
#define PRECALC_THETA		4


typedef struct {
	char *name;
	void (*fn)(Source *src, XformVariation *);
	int precalcFlags;
} Variation;

static inline void linear(XformVariation *xv, VarData *vd);
static inline void sinusoidal(XformVariation *xv, VarData *vd);
static inline void spherical(XformVariation *xv, VarData *vd);
static inline void swirl(XformVariation *xv, VarData *vd);
static inline void horseshoe(XformVariation *xv, VarData *vd);


static void emitLinear(Source *varSrc, XformVariation *xv);
static void emitSinusoidal(Source *varSrc, XformVariation *xv);
static void emitSpherical(Source *varSrc, XformVariation *xv);
static void emitSwirl(Source *varSrc, XformVariation *xv);
static void emitHorseshoe(Source *varSrc, XformVariation *xv);
static void emitPolar(Source *varSrc, XformVariation *xv);
static void emitHandkerchief(Source *varSrc, XformVariation *xv);
static void emitHeart(Source *varSrc, XformVariation *xv);
static void emitDisc(Source *varSrc, XformVariation *xv);

Variation variations[] = {
	{"linear", 			emitLinear,				PRECALC_NONE},
	{"sinusoidal",		emitSinusoidal,			PRECALC_NONE},
	{"spherical",		emitSpherical,			PRECALC_R_SQUARED},
	{"swirl",			emitSwirl,				PRECALC_R_SQUARED},
	{"horseshoe",		emitHorseshoe,			PRECALC_R},
	{"polar",			emitPolar,				PRECALC_R | PRECALC_THETA},
	{"handkerchief",	emitHandkerchief,		PRECALC_R | PRECALC_THETA},
	{"heart",			emitHeart,		 		PRECALC_R | PRECALC_THETA},
	{"disc",			emitDisc,		 		PRECALC_R | PRECALC_THETA}
	//{"spiral",			spiral},
	//{"hyperbolic",		hyperbolic}
};

void variationInit(Xform *xform, XformVariation *xv, int var, float weight, float p1, float p2, float p3, float p4) {
	xv->var = var;
	xv->weight = weight;
	xv->p1 = p1;
	xv->p2 = p2;
	xv->p3 = p3;
	xv->p4 = p4;

	xform->precalcFlags |= variations[var].precalcFlags;
}

void variationDo(Xform *xform, FLOAT *x, FLOAT *y) {
	VarData vd;
	vd.ox = *x;
	vd.oy = *y;
	vd.nx = 0.f;
	vd.ny = 0.f;

	// we can precalculate some commonly used values here
	// maybe always generate R squared?
	//if (xform->precalcFlags & PRECALC_R_SQUARED) {
	vd.rSquared = vd.ox * vd.ox + vd.oy * vd.oy;
	//}
	if (xform->precalcFlags & PRECALC_R) {
		vd.r = sqrtf(vd.rSquared);
	}	

	for (int i = 0; i < xform->nVars; i++) {
		XformVariation *xv = &xform->vars[i];
		switch (xv->var) {
			default:
			case 0:
				linear(xv, &vd);
				break;
			case 1: 
				sinusoidal(xv, &vd);
				break;
			case 2: 
				spherical(xv, &vd);
				break;
			case 3:
				swirl(xv, &vd);
				break;
			case 4:
				horseshoe(xv, &vd);
				break;
		}
		
	}

	*x = vd.nx;
	*y = vd.ny;
}

void variationGenerateSource(Source *src, Flame *flame) {
	Source *varSrc = sourceCreate();

	/*
	if (xform->precalcFlags & PRECALC_R) {
		vd.r = sqrtf(vd.rSquared);
	}*/	


	sourceAppend(varSrc, "\t\tx = 0; y = 0;\n");
	sourceAppend(varSrc, "\t\tswitch(xformIndex) {\n");	
	for (int xform = 0; xform < flame->nXforms; xform++) {
		sourceAppendFormatted(varSrc, "\t\t\tcase %d:\n", xform);
		Xform *xf = &flame->xforms[xform];
		sourceAppend(varSrc, "\t\t\t\t{\n");
		if (xf->precalcFlags & (PRECALC_R_SQUARED | PRECALC_R)) {
			sourceAppend(varSrc, "\t\t\t\tfloat rSquared = ox * ox + oy * oy;\n");
		}
		if (xf->precalcFlags & PRECALC_R) {
			sourceAppend(varSrc, "\t\t\t\tfloat r = native_sqrt(rSquared);\n");
		}
		if (xf->precalcFlags & PRECALC_THETA) {
			sourceAppend(varSrc, "\t\t\t\tfloat theta = atan(ox / (oy + PREVENT_DIVIDE_BY_ZERO));\n");
			//sourceAppend(varSrc, "\t\t\t\tfloat theta = 0.f;\n");
		}
		for (int j = 0; j < xf->nVars; j++) {
			int var = xf->vars[j].var;
			if (variations[var].fn != NULL) {


				sourceAppendFormatted(varSrc, "\t\t\t\t// variation %s (%d)\n", variations[var].name, var);
				variations[var].fn(varSrc, &xf->vars[j]);
			}	
	
		}
		sourceAppend(varSrc, "\t\t\t\t}\n");
		sourceAppend(varSrc, "\t\t\t\tbreak;\n");
	}
	sourceAppend(varSrc, "\t\t}\n");	

	sourceReplace(src, "// APPLY_VARIATIONS", varSrc->buffer);
	sourceDestroy(varSrc);
}


static inline void linear(XformVariation *xv, VarData *vd) {
	vd->nx += xv->weight * vd->ox;
	vd->ny += xv->weight * vd->oy;
}

static void emitLinear(Source *varSrc, XformVariation *xv) {
	sourceAppendFormatted(varSrc, "\t\t\t\tx += %.9ff * ox;\n", xv->weight);
	sourceAppendFormatted(varSrc, "\t\t\t\ty += %.9ff * oy;\n", xv->weight);
}

static inline void sinusoidal(XformVariation *xv, VarData *vd) {
	vd->nx += xv->weight * sinf(vd->ox);
	vd->ny += xv->weight * sinf(vd->oy);
}

static void emitSinusoidal(Source *varSrc, XformVariation *xv) {
	sourceAppendFormatted(varSrc, "\t\t\t\tx += %.9ff * native_sin(ox);\n", xv->weight);
	sourceAppendFormatted(varSrc, "\t\t\t\ty += %.9ff * native_sin(oy);\n", xv->weight);
}

static inline void spherical(XformVariation *xv, VarData *vd) {
	float a = xv->weight / (vd->rSquared + PREVENT_DIVIDE_BY_ZERO);
	vd->nx += a * vd->ox;
	vd->ny += a * vd->oy;
}

static void emitSpherical(Source *varSrc, XformVariation *xv) {
	sourceAppend(varSrc, "\t\t\t\t{\n");
	sourceAppendFormatted(varSrc, "\t\t\t\t\tfloat sphericalA = %.9ff / (rSquared + PREVENT_DIVIDE_BY_ZERO);\n", xv->weight);
	sourceAppend(varSrc, "\t\t\t\t\tx += sphericalA * ox;\n");
	sourceAppend(varSrc, "\t\t\t\t\ty += sphericalA * oy;\n");
	sourceAppend(varSrc, "\t\t\t\t}\n");
}

static inline void swirl(XformVariation *xv, VarData *vd) {
	float sinrs = sinf(vd->rSquared);
	float cosrs = cosf(vd->rSquared);
	vd->nx += xv->weight * (vd->ox * sinrs - vd->oy * cosrs);
	vd->ny += xv->weight * (vd->ox * cosrs + vd->oy * sinrs);
}

static void emitSwirl(Source *varSrc, XformVariation *xv) {
	sourceAppend(varSrc, "\t\t\t\t{\n");
	sourceAppend(varSrc, "\t\t\t\t\tfloat sinrs = native_sin(rSquared);\n");
	sourceAppend(varSrc, "\t\t\t\t\tfloat cosrs = native_cos(rSquared);\n");
	sourceAppendFormatted(varSrc, "\t\t\t\t\tx += %.9ff * (ox * sinrs - oy * cosrs);\n", xv->weight);
	sourceAppendFormatted(varSrc, "\t\t\t\t\ty += %.9ff * (ox * cosrs + oy * sinrs);\n", xv->weight);
	sourceAppend(varSrc, "\t\t\t\t}\n");
}

static inline void horseshoe(XformVariation *xv, VarData *vd) {
	float recipR = xv->weight / (vd->r + PREVENT_DIVIDE_BY_ZERO);
	vd->nx += recipR * (vd->ox * vd->ox - vd->oy * vd->oy);
	vd->ny += recipR * (2.f * vd->ox * vd->oy);
}


static void emitHorseshoe(Source *varSrc, XformVariation *xv) {
	sourceAppend(varSrc, "\t\t\t\t{\n");
	sourceAppendFormatted(varSrc, "\t\t\t\t\tfloat recipR = %.9ff / (r + PREVENT_DIVIDE_BY_ZERO);\n", xv->weight);
	sourceAppend(varSrc, "\t\t\t\t\tx += recipR * (ox * ox - oy * oy);\n");
	sourceAppend(varSrc, "\t\t\t\t\ty += recipR * (2.f * ox * oy);\n");
	sourceAppend(varSrc, "\t\t\t\t}\n");
}

static void emitPolar(Source *varSrc, XformVariation *xv) {
	sourceAppend(varSrc, "\t\t\t\t{\n");
	sourceAppendFormatted(varSrc, "\t\t\t\t\tx += %.9ff * (theta / M_PI_F);\n", xv->weight);
	sourceAppendFormatted(varSrc, "\t\t\t\t\ty += %.9ff * (r - 1);\n", xv->weight);
	sourceAppend(varSrc, "\t\t\t\t}\n");
}

static void emitHandkerchief(Source *varSrc, XformVariation *xv) {
	sourceAppend(varSrc, "\t\t\t\t{\n");
	sourceAppendFormatted(varSrc, "\t\t\t\t\tfloat rWeight = r * %.9ff;\n", xv->weight);
	sourceAppend(varSrc, "\t\t\t\t\tx += rWeight * native_sin(theta + r);\n");
	sourceAppend(varSrc, "\t\t\t\t\ty += rWeight * native_cos(theta - r);\n ");
	sourceAppend(varSrc, "\t\t\t\t}\n");
}

static void emitHeart(Source *varSrc, XformVariation *xv) {
	sourceAppend(varSrc, "\t\t\t\t{\n");
	sourceAppendFormatted(varSrc, "\t\t\t\t\tfloat rWeight = r * %.9ff;\n", xv->weight);
	sourceAppend(varSrc, "\t\t\t\t\tfloat rTheta = r * theta;\n");
	sourceAppend(varSrc, "\t\t\t\t\tx += rWeight * native_sin(rTheta);\n");
	sourceAppend(varSrc, "\t\t\t\t\ty -= rWeight * native_cos(rTheta);\n ");
	sourceAppend(varSrc, "\t\t\t\t}\n");
}

static void emitDisc(Source *varSrc, XformVariation *xv) {
	sourceAppend(varSrc, "\t\t\t\t{\n");
	sourceAppendFormatted(varSrc, "\t\t\t\t\tfloat thetaWeightPi = (theta * %.9ff) / M_PI_F;\n", xv->weight);
	sourceAppend(varSrc, "\t\t\t\t\tfloat rPi = r * M_PI_F;\n");
	sourceAppend(varSrc, "\t\t\t\t\tx += thetaWeightPi * native_sin(rPi);\n");
	sourceAppend(varSrc, "\t\t\t\t\ty += thetaWeightPi * native_cos(rPi);\n ");
	sourceAppend(varSrc, "\t\t\t\t}\n");
}

