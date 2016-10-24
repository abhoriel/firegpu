#ifndef FLAME_H
#define FLAME_H

#include "firegpu.h"
#include "palette.h"

typedef struct {
	Colour c;
	float count;
} Pixel;

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
	Palette *palette;
	Pixel *pixels;
} Flame;

#endif

