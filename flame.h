#ifndef FLAME_H
#define FLAME_H

#include "firegpu.h"
#include "xform.h"
#include "source.h"
#include "palette.h"

typedef struct __attribute__ ((packed)) {
	Colour c;
	float intensity;
} Pixel;

typedef struct {
	Xform *xforms;
	int nXforms;
	int supersample;
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


// a simplified flame structure which is used by the opencl kernel
typedef struct __attribute__ ((packed)) {
	int w;
	int h;
	int supersample;
	int iterations;
} FlameOpenCL;


Flame *flameCreate();
void flameDestroy(Flame *flame);
Xform *flameCreateXform(Flame *flame);
int flameGenerate(Flame *flame);
void flameTonemap(Flame *flame);
void flameDownsample(Flame *flame);
void flameRandomise(Flame *flame);
Source *flameGenerateSource(Flame *flame);

#endif

