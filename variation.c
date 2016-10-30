#include <math.h>
#include "firegpu.h"
#include "xform.h"
#include "rng.h"
#include "log.h"
#include "variation.h"

#define PREVENT_DIVIDE_BY_ZERO	0.0000000001f

#define PRECALC_NONE		0
#define PRECALC_R_SQUARED	1
#define PRECALC_R			2


typedef struct {
	char *name;
	void (*fn)(XformVariation *, VarData *);
	int precalcFlags;
} Variation;

static inline void linear(XformVariation *xv, VarData *vd);
static inline void sinusoidal(XformVariation *xv, VarData *vd);
static inline void spherical(XformVariation *xv, VarData *vd);
static inline void swirl(XformVariation *xv, VarData *vd);
static inline void horseshoe(XformVariation *xv, VarData *vd);

Variation variations[] = {
	{"linear", 			linear,				PRECALC_NONE},
	{"sinusoidal",		sinusoidal,			PRECALC_NONE},
	{"spherical",		spherical,			PRECALC_R_SQUARED},
	{"swirl",			swirl,				PRECALC_R_SQUARED},
	{"horseshoe",		horseshoe,			PRECALC_R}
	//{"polar",			polar},
	//{"handkerchief",	handkerchief},
	//{"heart",			heart},
	//{"disc",			disc},
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


static inline void linear(XformVariation *xv, VarData *vd) {
	vd->nx += xv->weight * vd->ox;
	vd->ny += xv->weight * vd->oy;
}

static inline void sinusoidal(XformVariation *xv, VarData *vd) {
	vd->nx += xv->weight * sinf(vd->ox);
	vd->ny += xv->weight * sinf(vd->oy);
}

static inline void spherical(XformVariation *xv, VarData *vd) {
	float a = xv->weight / (vd->rSquared + PREVENT_DIVIDE_BY_ZERO);
	vd->nx += a * vd->ox;
	vd->ny += a * vd->oy;
}

static inline void swirl(XformVariation *xv, VarData *vd) {
	float sinrs = sinf(vd->rSquared);
	float cosrs = cosf(vd->rSquared);
	vd->nx += xv->weight * (vd->ox * sinrs - vd->oy * cosrs);
	vd->ny += xv->weight * (vd->ox * cosrs + vd->oy * sinrs);
}

static inline void horseshoe(XformVariation *xv, VarData *vd) {
	float recipR = xv->weight / (vd->r + PREVENT_DIVIDE_BY_ZERO);
	vd->nx += recipR * (vd->ox * vd->ox - vd->oy * vd->oy);
	vd->ny += recipR * (2.f * vd->ox * vd->oy);
}


