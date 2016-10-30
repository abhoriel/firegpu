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

#endif // OPENCL_H

