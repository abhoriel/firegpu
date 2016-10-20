#ifndef XFORM_H
#define XFORM_H

#include "firegpu.h"

typedef struct {
	FLOAT a, b, c, d, e, f;
} Coefficients;

typedef struct {
	Coefficients coMain;
	Coefficients coFinal;

	int *vars;
	FLOAT *varCoefficients;
	int nVars;

	FLOAT weight;
	FLOAT symmetry;
	int colourIndex;
} Xform;


Xform *xformCreate();
void xformDestroy(Xform *xform);
void xformAddVariation(Xform *xform, int variation, FLOAT coefficient);

#endif
