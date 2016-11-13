#include <stdlib.h>
#include <math.h>
#include <assert.h>
#include "log.h"
#include "flame.h"
#include "filter.h"

static void generateKernels(Flame *flame);
static float epanechnikovKernel(float u);

void filterCreate(Flame *flame) {
	/*
	DensityEstimationFilter *def = malloc(sizeof(DensityEstimationFilter));
	if (def == NULL) {
		return NULL;
	}
	*/
	if (flame->def.kernels != NULL) {
		free(flame->def.kernels);
		flame->def.kernels = NULL;
	}
	generateKernels(flame);
}

static void generateKernels(Flame *flame) {
	//int maxWidthCeil = ceilf(def->maxRadius * ss * 2.f);
	//int nKernels = (def->maxRadius - def->minRadius) * 4;
	DensityEstimationFilter *def = &flame->def;
	float maxRadius = def->maxRadius * flame->supersample + 1;
	float minRadius = def->minRadius * flame->supersample + 1;
	int nKernels = (int)ceilf(powf(maxRadius / minRadius, 1.f / def->alpha));
	int rowSize = 2 * maxRadius - 1;
	int halfRowSize = (rowSize - 1) / 2;

	//plog(LOG_INFO, "maxRadius %f, minRadius %f, maxRadius / minRadius %f\n", maxRadius, minRadius, maxRadius / minRadius);
	plog(LOG_INFO, "nKernels %d, rowSize %d\n", nKernels, rowSize);
	def->kernels = malloc(nKernels * sizeof(float *));

	for (int i = 0; i < nKernels; i++) {
		//float radius = (def->minRadius + i * (((float)def->maxRadius - def->minRadius) / nKernels));
		float radius = maxRadius / powf(i + 1, def->alpha);
		//plog(LOG_INFO, "i %d, radius %.9f\n", i, radius);
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
				//plog(LOG_INFO, "%.5f ", kernel[y * rowSize + x]);
			}
			//plog(LOG_INFO, "\n");
		}
		def->kernels[i] = kernel;
		//plog(LOG_INFO, "\n\n");
	}
	def->nKernels = nKernels;
	def->rowSize = rowSize;
	def->halfRowSize = halfRowSize;
}

void filterDensityEstimation(Flame *flame) {
	DensityEstimationFilter *def = &flame->def;
	assert(def->nKernels > 0);
	float *filtered = calloc(flame->w * flame->h * flame->supersample * flame->supersample, sizeof(float));
	
	for (int y = 0; y < flame->h * flame->supersample; y += flame->supersample) {
		for (int x = 0; x < flame->w * flame->supersample; x += flame->supersample) {
			float sum = 0;
			for (int ssy = 0; ssy < flame->supersample; ssy++) {
				for (int ssx = 0; ssx < flame->supersample; ssx++) {
					Pixel *p = &flame->pixels[((y + ssy) * flame->w * flame->supersample) + x + ssx];
					sum += p->intensity;
				}
			}
			int kernel = floorf(powf(sum, def->alpha));
			if (kernel >= def->nKernels) {
				kernel = def->nKernels - 1;
			}

			for (int ky = 0; ky < def->rowSize; ky++) {
				for (int kx = 0; kx < def->rowSize; kx++) {
					int sy = ky - def->halfRowSize;
					int sx = kx - def->halfRowSize;

					float sample = def->kernels[kernel][ky * def->rowSize + kx];

					if (((y + sy) < 0) || ((y + sy) >= (flame->h * flame->supersample))) {
						continue;
					}
					if (((x + sx) < 0) || ((x + sx) >= (flame->w * flame->supersample))) {
						continue;
					}
					filtered[(y + sy) * flame->w * flame->supersample + x + sx] += sample;
				}
			}
		}
	}

	for (int y = 0; y < flame->h * flame->supersample; y++) {
		for (int x = 0; x < flame->w * flame->supersample; x++) {
			Pixel *p = &flame->pixels[y * flame->w * flame->supersample + x];
			p->intensity = filtered[y * flame->w * flame->supersample + x];
		}
	}
	free(filtered);
}

static float epanechnikovKernel(float u) {
	return 0.75f * (1.f - (u * u));
}

