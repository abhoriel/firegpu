#ifndef XFORM_H
#define XFORM_H

#include "firegpu.h"

typedef struct {
	FLOAT a, b, c, d, e, f;
} Coefficients;

typedef struct {
	int var;
	float weight;
	float p1, p2, p3, p4;
} XformVariation;

typedef struct {
	Coefficients coMain;
	Coefficients coFinal;

	int hasFinal;

	//int *vars;
	//FLOAT *varWeights;
	XformVariation *vars;
	int nVars;

	FLOAT weight;
	FLOAT symmetry;
	FLOAT opacity;
	FLOAT colour;
} Xform;


Xform *xformCreate();
void xformDestroy(Xform *xform);
void xformAddVariation(Xform *xform, int variation, FLOAT weight);

#endif
