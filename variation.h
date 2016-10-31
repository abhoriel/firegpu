#ifndef VARIATION_H
#define VARIATION_H

#include "firegpu.h"
#include "flame.h"
#include "xform.h"
#include "source.h"

void variationInit(Xform *xform, XformVariation *xv, int var, float weight, float p1, float p2, float p3, float p4);
void variationDo(Xform *xform,  FLOAT *x, FLOAT *y);
void variationGenerateSource(Source *src, Flame *flame);

typedef struct {
	FLOAT ox, oy;
	FLOAT nx, ny;

	FLOAT rSquared, r, theta, phi;
	int preCalcFlags;
} VarData;

#endif
