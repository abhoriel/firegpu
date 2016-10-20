#ifndef FLAME_H
#define FLAME_H

#include "firegpu.h"

typedef struct {
	Xform *xforms;
	int nXforms;
	int superSample;
	int w, h;
	int iterations;
	int quality;
	FLOAT centreX, centreY;
	FLOAT scale;
	FLOAT rotate;
	FLOAT vibrancy, gamma, hue;
	FLOAT *histogram;
} Flame;

#endif

