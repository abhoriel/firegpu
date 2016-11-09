#include <stdlib.h>
#include <math.h>
#include <assert.h>
#include "log.h"
#include "flame.h"
#include "filter.h"

static void generateKernels(DensityEstimationFilter *def);
static float epanechnikovKernel(float u);

DensityEstimationFilter *filterCreate(float minRadius, float maxRadius, float alpha) {
	DensityEstimationFilter *def = malloc(sizeof(DensityEstimationFilter));
	if (def == NULL) {
		return NULL;
	}
	def->alpha = alpha;
	def->minRadius = minRadius;
	def->maxRadius = maxRadius;
	generateKernels(def);
	return def;
}

static void generateKernels(DensityEstimationFilter *def) {
	int maxWidthCeil = ceilf(def->maxRadius * 2.f);
	int nKernels = (def->maxRadius - def->minRadius) * 4;
	float maxRadius = def->maxRadius;

	def->kernels = malloc(nKernels * sizeof(float *));
	assert(def->maxRadius > 0);
	assert(def->minRadius > 0);

	for (int i = 0; i < nKernels; i++) {
		float radius = (def->minRadius + i * (((float)def->maxRadius - def->minRadius) / nKernels));
		plog(LOG_INFO, "radius %.9f, maxRadius %f, nKernels %d\n", radius, maxRadius, nKernels);
		float *kernel = malloc(maxWidthCeil * maxWidthCeil * sizeof(float));
		float total = 0.f;

		for (int y = 0; y < maxWidthCeil; y++) {
			for (int x = 0; x < maxWidthCeil; x++) {
				float yf = y - maxRadius;
				float xf = x - maxRadius;
				float mag = sqrtf(xf * xf + yf * yf);
				if (mag > radius) {
					mag = radius;
				}
				if (radius > 0) {
					total += epanechnikovKernel(mag / radius);
				}
			}
		}

		for (int y = 0; y < maxWidthCeil; y++) {
			for (int x = 0; x < maxWidthCeil; x++) {
				float yf = y - maxRadius;
				float xf = x - maxRadius;
				float mag = sqrtf(xf * xf + yf * yf);
				if (mag > radius) {
					mag = radius;
				}
				if (radius == 0) {
					kernel[y * maxWidthCeil + x] = 0.f;
				} else {
					kernel[y * maxWidthCeil + x] = epanechnikovKernel(mag / radius) / total;
				}
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

