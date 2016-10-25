#ifndef FLAME_H
#define FLAME_H

#include "firegpu.h"
#include "xform.h"
#include "palette.h"

typedef struct {
	Colour c;
	float intensity;
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

Flame *flameCreate();
void flameDestroy(Flame *flame);
Xform *flameCreateXform(Flame *flame);
int flameGenerate(Flame *flame);
void flameTonemap(Flame *flame);
void flameDownsample(Flame *flame);

#endif

