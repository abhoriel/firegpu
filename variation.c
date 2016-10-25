#include <math.h>
#include "firegpu.h"
#include "xform.h"
#include "rng.h"
#include "log.h"
#include "variation.h"

typedef struct {
	char *name;
	void (*fn)(XformVariation *, VarData *);
} Variation;

static void linear(XformVariation *xv, VarData *vd);
static void sinusoidal(XformVariation *xv, VarData *vd);
static void spherical(XformVariation *xv, VarData *vd);

Variation variations[] = {
	{"linear", 			linear},
	{"sinusoidal",		sinusoidal},
	{"spherical",		spherical}
	//{"swirl",			swirl},
	//{"horseshoe",		horseshoe},
	//{"polar",			polar},
	//{"handkerchief",	handkerchief},
	//{"heart",			heart},
	//{"disc",			disc},
	//{"spiral",			spiral},
	//{"hyperbolic",		hyperbolic}

};


void variationInit() {
	
}


void variationDo(Xform *xform, FLOAT *x, FLOAT *y) {
	VarData vd;
	vd.ox = *x;
	vd.oy = *y;
	vd.nx = 0.f;
	vd.ny = 0.f;

	for (int i = 0; i < xform->nVars; i++) {
		XformVariation *xv = &xform->vars[i];
		variations[xv->var].fn(xv, &vd);
		
	}

	*x = vd.nx;
	*y = vd.ny;

		// this way is probably more efficient but i cant be bothered
		// I guess it depends on the compiler and instruction cache
		/*
		switch (xform->vars[i]) {
			case 0:
				linear(xform, x, y);
				break;
			case 1: 

				break;
			default:
				plog(LOG_ERROR, "unknown variation %d\n", i);
		}
		*/
}

//static void linear(Xform *xform, FLOAT *x, FLOAT *y) {
static void linear(XformVariation *xv, VarData *vd) {
	vd->nx += xv->weight * vd->ox;
	vd->ny += xv->weight * vd->oy;
}

static void sinusoidal(XformVariation *xv, VarData *vd) {
	vd->nx += xv->weight * sinf(vd->ox);
	vd->ny += xv->weight * sinf(vd->oy);
}

static void spherical(XformVariation *xv, VarData *vd) {
	float a = xv->weight / (vd->ox * vd->ox + vd->oy * vd->oy + 0.00000000001f);
	vd->nx += a * vd->ox;
	vd->ny += a * vd->oy;
}


