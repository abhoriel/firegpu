#ifndef VARIATION_H
#define VARIATION_H

#include "firegpu.h"
#include "xform.h"


void variationDo(Xform *xform,  FLOAT *x, FLOAT *y);

typedef struct {
	FLOAT ox, oy;
	FLOAT nx, ny;

	FLOAT rPreCalc, thetaPreCalc, phyPreCalc;
} VarData;


#endif
