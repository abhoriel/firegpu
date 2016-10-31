#ifndef OPENCL_H
#define OPENCL_H

#include "flame.h"
#include "palette.h"

int openclInit(int desiredPlatform, int desiredDevice);
void openclFini();
int openclBuildProgram(const char *source);
void openclFiniProgram();
int openclExecMandlebrot(int w, int h, double scale, double topLeftX, double topLeftY, int limit, double *samples);

int openclExecFlame(Flame *flame, int *xfd, int xfdSize, Pixel *pixels, int nPixels);

// a simplified flame structure which is used by the opencl kernel
typedef struct __attribute__ ((packed)) {
	int w;
	int h;
	int supersample;
	int iterations;
	int seed;
} FlameOpenCL;

typedef struct __attribute__ ((packed)) {
	Colour colour;
	float a, b, c, d, e, f;
	float opacity;
} XformOpenCL;

#endif // OPENCL_H

