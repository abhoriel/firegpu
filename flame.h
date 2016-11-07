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
	float centreX, centreY;
	float scale;
	float rotate;
	float vibrancy, gamma, hue;
	float maxKernelRadius, minKernelRadius, alpha; 
	Palette *palette;
	Pixel *pixels;
} Flame;


Flame *flameCreate();
void flameDestroy(Flame *flame);
Xform *flameCreateXform(Flame *flame);
int flameGenerate(Flame *flame);
void flameTonemap(Flame *flame);
void flameDownsample(Flame *flame);
void flameRandomise(Flame *flame);
Source *flameGenerateSource(Flame *flame);

#endif

