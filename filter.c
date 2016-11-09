#include <stdlib.h>
#include <math.h>
#include <assert.h>
#include "log.h"
#include "flame.h"
#include "filter.h"

static void generateKernels(DensityEstimationFilter *def);
static float epanechnikovKernel(float u);

DensityEstimationFilter *filterCreate(float minWidth, float maxWidth, float alpha) {
	DensityEstimationFilter *def = malloc(sizeof(DensityEstimationFilter));
	if (def == NULL) {
		return NULL;
	}
	def->alpha = alpha;
	def->minWidth = minWidth;
	def->maxWidth = maxWidth;
	generateKernels(def);
	return def;
}

static void generateKernels(DensityEstimationFilter *def) {
	int maxWidthCeil = ceilf(def->maxWidth);
	int nKernels = (def->maxWidth - def->minWidth) + 1;
	float maxRadius = def->maxWidth / 2.f;

	def->kernels = malloc(nKernels * sizeof(float *));
	assert(def->maxWidth > 0);
	assert(def->minWidth >= 0);

	for (int i = 0; i < nKernels; i++) {
		float radius = def->minWidth + i * (((float)def->maxWidth - def->minWidth) / (nKernels * 2.f));
		float *kernel = malloc(maxWidthCeil * maxWidthCeil * sizeof(float));
		for (int y = 0; y < maxWidthCeil; y++) {
			for (int x = 0; x < maxWidthCeil; x++) {
				float yf = y - maxRadius;
				float xf = x - maxRadius;
				float mag = sqrtf(xf * xf + yf * yf);
				if (mag > radius) {
					mag = radius;
				}
				kernel[y * maxWidthCeil + x] = epanechnikovKernel(mag / radius);	
				plog(LOG_INFO, "%.5f ", kernel[y * maxWidthCeil + x]);
			}
			plog(LOG_INFO, "\n");
		}
		def->kernels[i] = kernel;
		plog(LOG_INFO, "\n\n");
	}
	def->nKernels = nKernels;
}


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


static float epanechnikovKernel(float u) {
	return 0.75f * (1.f - (u * u));
}


