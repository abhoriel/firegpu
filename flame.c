#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <math.h>
#include "log.h"
#include "xform.h"
#include "rng.h"
#include "palette.h"
#include "variation.h"
#include "flame.h"

// MUST be a power of 2
#define XFORM_DISTRIBUTION_SIZE	4096

static int *createXformDistribution(Flame *flame);
static int flameGetNSamples(Flame *flame);

Flame *flameCreate() {
	Flame *flame = malloc(sizeof(Flame));
	flame->xforms = NULL;
	flame->nXforms = 0;
	flame->pixels = NULL;
	flame->palette = paletteCreate();
	return flame;
}

void flameDestroy(Flame *flame) {
	for (int i = 0; i < flame->nXforms; i++) {
		xformFini(&flame->xforms[i]);
	}
	if (flame->xforms != NULL) {
		free(flame->xforms);
	}
	paletteDestroy(flame->palette);
	if (flame->pixels != NULL) {
		free(flame->pixels);
	}
	free(flame);
}

// totally broken
Xform *flameCreateXform(Flame *flame) {
	flame->xforms = realloc(flame->xforms, sizeof(Xform) * (flame->nXforms + 1));
	if (flame->xforms == NULL) {
		plog(LOG_ERROR, "flameCreateXform(): memory allocation failed\n");
		return NULL;
	}
	Xform *xform = &flame->xforms[flame->nXforms];
	xformInit(xform);

	flame->nXforms++;
	return xform;
}


int flameGenerate(Flame *flame) {
	if (flame->pixels != NULL) {
		free(flame->pixels);
		flame->pixels = NULL;
	}

	int nSamples = flameGetNSamples(flame);
	//float *histogram = malloc(nSamples * sizeof(float));
	//Colour *colours = malloc(nSamples * sizeof(Colour));
	Pixel *pixels = malloc(nSamples * sizeof(Pixel));
	if (pixels == NULL) {
		plog(LOG_ERROR, "drawFlame(): memory allocation failure\n");
		return -1;
	}
	for (int i = 0; i < nSamples; i++) {
		pixels[i].intensity = 0.f;
		pixels[i].c.r = 0.f;
		pixels[i].c.g = 0.f;
		pixels[i].c.b = 0.f;
	}

	int *xfd = createXformDistribution(flame);
	int quality = flame->quality * flame->w * flame->h;
	for (int sample = 0; sample < quality; sample++) {
		float x = rngGenerateFloat(-1.f, 1.f);
		float y = rngGenerateFloat(-1.f, 1.f);
		//float colour = rngGenerateFloat(0.f, 1.f);
		//float alpha = rngGenerateFloat(0.f, 1.f);

		for (int j = -20; j < flame->iterations; j++) {
			// decide which xform to use
			int xformIndex = xfd[rngGenerate32() & (XFORM_DISTRIBUTION_SIZE - 1)];
			Xform *xform = &flame->xforms[xformIndex];
			//float alpha = xform->opacity;
			
			// in opencl we can use fma here
			float newx = x * xform->coMain.a + y * xform->coMain.b + xform->coMain.c;
			float newy = x * xform->coMain.d + y * xform->coMain.e + xform->coMain.f;
			
			variationDo(xform, &newx, &newy);

			if (xform->hasFinal) {
				x = newx * xform->coFinal.a + newy + xform->coFinal.b + xform->coFinal.c;
				y = newx * xform->coFinal.d + newy + xform->coFinal.e + xform->coFinal.f;
				// there can be a variation here too..
			} else {
				x = newx;
				y = newy;
			}

			if (j > 0) {
				int xi = (x + 1.f) * 0.5f * flame->w * flame->supersample;
				int yi = (y + 1.f) * 0.5f * flame->h * flame->supersample;
				if ((xi < 0) || (xi >= flame->w) || (yi < 0) || (yi >= flame->h)) {
					continue;
				}
				Pixel *pixel = &pixels[xi + (yi * flame->w * flame->supersample)];
				//Colour *colour = &pixels[xi + (yi * flame->w * flame->superSample)].c;
				//if (histogram[xi + (yi * flame->w * flame->superSample)] == 0.f) {
					//colour->r = xform->
				//}
				Colour temp;
				paletteGetColour(flame->palette, xform->colour, &temp);
				// we can use the count to do all these divides at the end
				pixel->c.r += temp.r * xform->opacity;
				pixel->c.g += temp.g * xform->opacity;
				pixel->c.b += temp.b * xform->opacity;

				pixel->intensity += xform->opacity; 
			}
		}
	}

	free(xfd);
	flame->pixels = pixels;
	return 0;
}

// 
static int *createXformDistribution(Flame *flame) {
	assert(flame->nXforms > 0);
	int *xfd = malloc(sizeof(int) * XFORM_DISTRIBUTION_SIZE);
	int xform = 0;
	float offset = 0;
	float total = 0;
	for (int i = 0; i < flame->nXforms; i++) {
		total += flame->xforms[xform].weight;
	}
	for (int i = 0; i < XFORM_DISTRIBUTION_SIZE; i++) {
		//float p = ((float)i / (XFORM_DISTRIBUTION_SIZE * total));
		float p = (((float)i) * total) / XFORM_DISTRIBUTION_SIZE;
		if (((p - offset) > flame->xforms[xform].weight) && (xform < (flame->nXforms - 1))) {
			xform++;
			offset += p;
		}
		xfd[i] = xform;
	}
	return xfd;
}

static int flameGetNSamples(Flame *flame) {
	return flame->w * flame->supersample * flame->h * flame->supersample;
}

void flameTonemap(Flame *flame) {
	assert(flame->pixels != NULL);
	FLOAT max = 0;
	int nSamples = flameGetNSamples(flame);
	for (int i = 0; i < nSamples; i++) {
		// divide by the intensity as we need the average colours here.
		flame->pixels[i].c.r /= flame->pixels[i].intensity;
		flame->pixels[i].c.g /= flame->pixels[i].intensity;
		flame->pixels[i].c.b /= flame->pixels[i].intensity;
		flame->pixels[i].intensity = log10f(flame->pixels[i].intensity);
		if (flame->pixels[i].intensity > max) {
			max = flame->pixels[i].intensity;
		}
	}
	for (int i = 0; i < nSamples; i++) {
		// gamma adjustment
		flame->pixels[i].intensity = powf(flame->pixels[i].intensity / max, 1.0f / flame->gamma);
		flame->pixels[i].c.r *= flame->pixels[i].intensity;
		flame->pixels[i].c.g *= flame->pixels[i].intensity;
		flame->pixels[i].c.b *= flame->pixels[i].intensity;
	}
}

// this function is destructive, in that it reuses the same pixel buffer
void flameDownsample(Flame *flame) {
	int supersample = flame->supersample;
	assert(flame->pixels != NULL);
	for (int y = 0; y < flame->h; y++) {
		for (int x = 0; x < flame->w; x++) {
			float totalIntensity = 0.f;
			Colour total;
			total.r = 0.f;
			total.g = 0.f;
			total.b = 0.f;
			for (int yss = 0; yss < supersample; yss++) {
				for (int xss = 0; xss < supersample; xss++) {
					Pixel *pixel = &flame->pixels[(y * supersample * flame->w) + (x * supersample) + (yss * flame->w) + xss];
					totalIntensity += pixel->intensity;
					total.r += pixel->c.r;
					total.g += pixel->c.g;
					total.b += pixel->c.b;
				}
			}
			Pixel *pixel = &flame->pixels[(y * flame->w) + x];
			pixel->intensity = totalIntensity / (supersample * supersample);
			pixel->c.r = total.r / (supersample * supersample);
			pixel->c.g = total.g / (supersample * supersample);
			pixel->c.b = total.b / (supersample * supersample);
		}
	} 
}

