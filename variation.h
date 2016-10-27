#ifndef VARIATION_H
#define VARIATION_H

#include "firegpu.h"
#include "xform.h"

void variationInit(Xform *xform, XformVariation *xv, int var, float weight, float p1, float p2, float p3, float p4);
void variationDo(Xform *xform,  FLOAT *x, FLOAT *y);

typedef struct {
	FLOAT ox, oy;
	FLOAT nx, ny;

	FLOAT rSquared, theta, phi;
	int preCalcFlags;
} VarData;


#endif
