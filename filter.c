#include <stdlib.h>
#include <math.h>
#include <assert.h>
#include "log.h"
#include "flame.h"
#include "filter.h"

static void generateKernels(DensityEstimationFilter *def, int ss);
static float epanechnikovKernel(float u);

DensityEstimationFilter *filterCreate(float minRadius, float maxRadius, float alpha, int ss) {
	DensityEstimationFilter *def = malloc(sizeof(DensityEstimationFilter));
	if (def == NULL) {
		return NULL;
	}
	def->alpha = alpha;
	def->minRadius = minRadius;
	def->maxRadius = maxRadius;
	generateKernels(def, ss);
	return def;
}

static void generateKernels(DensityEstimationFilter *def, int ss) {
	//int maxWidthCeil = ceilf(def->maxRadius * ss * 2.f);
	//int nKernels = (def->maxRadius - def->minRadius) * 4;
	float maxRadius = def->maxRadius * ss + 1;
	float minRadius = def->minRadius * ss + 1;
	int nKernels = (int)ceilf(powf(maxRadius / minRadius, 1.f / def->alpha));
	int rowSize = 2 * maxRadius - 1;
	int halfRowSize = (rowSize - 1) / 2;

	plog(LOG_INFO, "maxRadius %f, minRadius %f, maxRadius / minRadius %f\n", maxRadius, minRadius, maxRadius / minRadius);
	plog(LOG_INFO, "nKernels %d, rowSize %d\n", nKernels, rowSize);
	def->kernels = malloc(nKernels * sizeof(float *));

	for (int i = 0; i < nKernels; i++) {
		//float radius = (def->minRadius + i * (((float)def->maxRadius - def->minRadius) / nKernels));
		float radius = maxRadius / powf(i + 1, def->alpha);
		plog(LOG_INFO, "i %d, radius %.9f\n", i, radius);
		float *kernel = malloc(rowSize * rowSize * sizeof(float));
		float total = 0.f;

		for (int y = 0; y < rowSize; y++) {
			for (int x = 0; x < rowSize; x++) {
				float yf = y - halfRowSize;
				float xf = x - halfRowSize;
				float mag = sqrtf(xf * xf + yf * yf);
				if (mag > radius) {
					mag = radius;
				}
				if (radius > 0) {
					total += epanechnikovKernel(mag / radius);
				}
			}
		}

		for (int y = 0; y < rowSize; y++) {
			for (int x = 0; x < rowSize; x++) {
				float yf = y - halfRowSize;
				float xf = x - halfRowSize;
				float mag = sqrtf(xf * xf + yf * yf);
				if (mag > radius) {
					mag = radius;
				}
				if (radius == 0) {
					kernel[y * rowSize + x] = 0.f;
				} else {
					kernel[y * rowSize + x] = epanechnikovKernel(mag / radius) / total;
				}
				plog(LOG_INFO, "%.5f ", kernel[y * rowSize + x]);
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

