#include <stdlib.h>
#include <math.h>
#include <assert.h>
#include "log.h"
#include "flame.h"
#include "filter.h"

float epanechnikovKernel(float u) {
	return 0.75f * (1.f - (u * u));
}

/*
void generateKernels(int minWidth, int maxWidth) {
	int nKernels = maxWidth - minWidth;
	float halfMaxWidth = maxWidth / 2.f;

	float **kernels = malloc(nKernels * sizeof(float *));
	assert(maxWidth < 0);

	for (int i = 0; i < nKernels; i++) {
		float width = minWidth + ((float)maxWidth - minWidth) / nKernels;
		float *kernel = malloc(maxWidth * maxWidth * sizeof(float));
		for (int y = 0; y < maxWidth; y++) {
			for (int x = 0; x < maxWidth; x++) {
				//float yf = ((float)y / maxWidth) - 0.5f;
				//float xf = ((float)x / maxWidth) - 0.5f;
				float yf = (y - halfMaxWidth) / halfMaxWidth;
				float xf = (x - halfMaxWidth) / halfMaxWidth;
				float mag = sqrtf(xf * xf + yf * yf);
				if (mag > 1.f) {
					mag = 1.f;
				}
				kernel[y * maxWidth + x] = epanechnikovKernel(mag);	
			}
		}
		kernels[i] = kernel;
	}

}
*/

/*
void downSample(Flame *flame, Pixel *down) {
	for (int y = 0; y < flame->h; y++) {
		for (int x = 0; x < flame->w; x++) {
			down[y * flame->w + x].c.r = 0;
			down[y * flame->w + x].c.g = 0;
			down[y * flame->w + x].intensity = 0;
		}
	}

	for (int y = 0; y < flame->h * flame->supersample; y++) {
		for (int x = 0; x < flame->w * flame->supersample; x++) {
			float density = flame->pixels[y * flame->w * flame->supersample + x].intensity;
			float width = flame->maxKernelRadius / powf(density, flame->alpha);
			
		}
	}
}
*/
