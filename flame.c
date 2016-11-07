#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <math.h>
#include "log.h"
#include "xform.h"
#include "rng.h"
#include "palette.h"
#include "variation.h"
#include "source.h"
#include "opencl.h"
#include "flame.h"

// MUST be a power of 2
#define XFORM_DISTRIBUTION_SIZE	4096

static int *createXformDistribution(Flame *flame);
static int flameGetNSamples(Flame *flame);
static void flameGenerateCpu(Flame *flame, int *xfd, Pixel *pixels);
static void flameSetXformColours(Flame *flame);

Flame *flameCreate() {
	Flame *flame = malloc(sizeof(Flame));
	flame->xforms = NULL;
	flame->nXforms = 0;
	flame->pixels = NULL;
	flame->palette = paletteCreate();
	flame->alpha = 0.4f;
	flame->maxKernelRadius = 9.0f;
	flame->minKernelRadius = 0.f;
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
	flameSetXformColours(flame);

	int *xfd = createXformDistribution(flame);

	int ret;
	if (1) {
		ret = openclExecFlame(flame, xfd, XFORM_DISTRIBUTION_SIZE, pixels, nSamples);
	} else {
		flameGenerateCpu(flame, xfd, pixels);
		ret = 0;
	}

	free(xfd);
	flame->pixels = pixels;
	return ret;
}

// reference CPU implementation. slow, does not use threading
static void flameGenerateCpu(Flame *flame, int *xfd, Pixel *pixels) {
	int quality = flame->quality * flame->w * flame->h;

	for (int sample = 0; sample < quality; sample++) {

		if ((sample % 1000) == 0) {
			plog(LOG_INFO, "\rprogress %f%%...", 100.f * (float)sample / quality);
		}

		float x = rngGenerateFloat(-1.f, 1.f);
		float y = rngGenerateFloat(-1.f, 1.f);

		for (int j = -20; j < flame->iterations; j++) {
			// decide which xform to use
			int xformIndex = xfd[rngGenerate32() & (XFORM_DISTRIBUTION_SIZE - 1)];
			Xform *xform = &flame->xforms[xformIndex];
			
			// in opencl we can use fma here
			float newx = x * xform->coMain.a + y * xform->coMain.b + xform->coMain.c;
			float newy = x * xform->coMain.d + y * xform->coMain.e + xform->coMain.f;
			
			variationDo(xform, &newx, &newy);

			if (xform->hasPost) {
				x = newx * xform->coPost.a + newy + xform->coPost.b + xform->coPost.c;
				y = newx * xform->coPost.d + newy + xform->coPost.e + xform->coPost.f;
				// there can be a variation here too..
			} else {
				x = newx;
				y = newy;
			}

			// after the first 20 iterations, we plot the points
			if (j > 0) {
				int xi = (x + 1.f) * 0.5f * flame->w * flame->supersample;
				int yi = (y + 1.f) * 0.5f * flame->h * flame->supersample;
				if ((xi < 0) || (xi >= (flame->w * flame->supersample)) || (yi < 0) || (yi >= (flame->h * flame->supersample))) {
					continue;
				}
				Pixel *pixel = &pixels[xi + (yi * flame->w * flame->supersample)];

				pixel->c.r += xform->colour.r * xform->opacity;
				pixel->c.g += xform->colour.g * xform->opacity;
				pixel->c.b += xform->colour.b * xform->opacity;

				pixel->intensity += xform->opacity; 
			}
		}
	}
	plog(LOG_INFO, "\t");
}

Source *flameGenerateSource(Flame *flame) {
	flameSetXformColours(flame);

	Source *src = sourceLoad("flamekernel.cl");
	if (src == NULL) {
		return NULL;
	}

	sourceReplaceFormatted(src, "XFORM_DISTRIBUTION_SIZE", "%u", XFORM_DISTRIBUTION_SIZE);

	/*
	Source *xformSrc = sourceCreate();	
	for (int i = 0; i < flame->nXforms; i++) {
		sourceAppendFormatted(xformSrc,	
						"\t\t\tcase %d:\n"
						"\t\t\t\tnewX = x * %.9ff + y * %.9ff + %.9ff;\n"
						"\t\t\t\tnewY = x * %.9ff + y * %.9ff + %.9ff;\n"
						"\t\t\t\txformColourR = %.9ff;\n"
						"\t\t\t\txformColourG = %.9ff;\n"
						"\t\t\t\txformColourB = %.9ff;\n"
						"\t\t\t\txformOpacity = %.9ff;\n",
					i,	flame->xforms[i].coMain.a, flame->xforms[i].coMain.b,
			  			flame->xforms[i].coMain.c, flame->xforms[i].coMain.d,
						flame->xforms[i].coMain.e, flame->xforms[i].coMain.f,
						flame->xforms[i].colour.r,
						flame->xforms[i].colour.g,
						flame->xforms[i].colour.b,
						flame->xforms[i].opacity
						);
		sourceAppend(xformSrc, "\t\t\t\tbreak;\n");
	}
	sourceReplace(src, "// XFORM_SWITCH", xformSrc->buffer);
	sourceDestroy(xformSrc);
	*/

	variationGenerateSource(src, flame);

	return src;
}

// to accelerate randomly choosing an xform during each iteration, an array
// containing a distribution of xforms according to their weights (ie. 
// relative probability) is created. judging from the flames i have seen,
// the weights of all xforms does not necessarily add up to 1.
// XFORM_DISTRIBUTION_SIZE affects the resolution of the distribution, with
// larger differences in relative probability being more accurately represented
// the caller is responsible for free()ing the distribution
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

// apply log density, normalise and gamma correction
void flameTonemap(Flame *flame) {
	assert(flame->pixels != NULL);
	float max = 0;
	for (int y = 0; y < (flame->h * flame->supersample); y++) {
		for (int x = 0; x < (flame->w * flame->supersample); x++) {
			Pixel *pixel = &flame->pixels[y * flame->w * flame->supersample + x];
			// divide by the intensity as we need the average colours here.
			if (pixel->intensity != 0) {
				pixel->c.r /= pixel->intensity;
				pixel->c.g /= pixel->intensity;
				pixel->c.b /= pixel->intensity;
				pixel->intensity = log10f(pixel->intensity);
				if (pixel->intensity > max) {
					max = pixel->intensity;
				}
			}
		}
	}
	for (int y = 0; y < (flame->h * flame->supersample); y++) {
		for (int x = 0; x < (flame->w * flame->supersample); x++) {
			// gamma adjustment
			Pixel *pixel = &flame->pixels[y * flame->w * flame->supersample + x];
			pixel->intensity = powf(pixel->intensity / max, 1.0f / flame->gamma);
			pixel->c.r *= pixel->intensity;
			pixel->c.g *= pixel->intensity;
			pixel->c.b *= pixel->intensity;
			// mathematically, this shouldnt happen. but due to non-atomic arithmetic and 
			// race conditions in the GPU code, it can
			if (pixel->c.r > 1.f) {
				pixel->c.r = 1.f;
			}
			if (pixel->c.g > 1.f) {
				pixel->c.g = 1.f;
			}
			if (pixel->c.b > 1.f) {
				pixel->c.b = 1.f;
			}
			/*

			*/
		}
	}
}

// down sample the flame using mean average
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
					Pixel *pixel = &flame->pixels[((y * supersample) + yss) * supersample * flame->w + x * supersample + xss];
					totalIntensity += pixel->intensity;
					total.r += pixel->c.r;
					total.g += pixel->c.g;
					total.b += pixel->c.b;
				}
			}
			Pixel *pixel = &flame->pixels[(y * flame->w * supersample) + x];
			pixel->intensity = totalIntensity;
			pixel->c.r = total.r / (supersample * supersample);
			pixel->c.g = total.g / (supersample * supersample);
			pixel->c.b = total.b / (supersample * supersample);
		}
	} 
}

// randomise the flame, destroys most of the previous flame parameters
void flameRandomise(Flame *flame) {
	for (int i = 0; i < flame->nXforms; i++) {
		xformFini(&flame->xforms[i]);
	}
	flame->nXforms = 0;
	if (flame->xforms != NULL) {
		free(flame->xforms);
	}
	flame->xforms = NULL;
	
	int nXforms = (rngGenerate32() % 5) + 1;
	plog(LOG_INFO, "\n\nnew fractal:\n");
	for (int i = 0; i < nXforms; i++) {
		Xform *xform = flameCreateXform(flame);
		xform->hasPost = 1;
		xform->weight = rngGenerateFloat(0.f, 1.f);
		xform->colourIndex = rngGenerateFloat(0.f, 1.f);
		xform->opacity = 1.0f;
		xform->symmetry = 0.0f;
		xform->coMain.a = rngGenerateFloat(-1.f, 1.f); 
		xform->coMain.b = rngGenerateFloat(-1.f, 1.f); 
		xform->coMain.c = rngGenerateFloat(-1.f, 1.f); 
		xform->coMain.d = rngGenerateFloat(-1.f, 1.f); 
		xform->coMain.e = rngGenerateFloat(-1.f, 1.f); 
		xform->coMain.f = rngGenerateFloat(-1.f, 1.f); 

		xform->coPost.a = rngGenerateFloat(-1.f, 1.f); 
		xform->coPost.b = rngGenerateFloat(-1.f, 1.f); 
		xform->coPost.c = rngGenerateFloat(-1.f, 1.f); 
		xform->coPost.d = rngGenerateFloat(-1.f, 1.f); 
		xform->coPost.e = rngGenerateFloat(-1.f, 1.f); 
		xform->coPost.f = rngGenerateFloat(-1.f, 1.f); 

		plog(LOG_INFO, "new xform: weight %f, colour %f, a %.9g, b %.9g, c %.9g, d %.9g, e %.9g, f %.9g\n", xform->weight, xform->colourIndex, xform->coMain.a, xform->coMain.b, xform->coMain.c, xform->coMain.d, xform->coMain.e, xform->coMain.f);
		int nVars = (rngGenerate32() % 3) + 1;
		float total = 0.f;
		int *varsDone = calloc(N_VARIATIONS, sizeof(int));

		for (int j = 0; j < nVars; j++) {
			float weight;
			int var;
			do {
				var = rngGenerate32() % N_VARIATIONS;
			} while(varsDone[var] != 0);
			varsDone[var] = 1;
			if (j != (nVars - 1)) {
				weight = rngGenerateFloat(0.0f, 1.f - total);
				total += weight;
			} else {
				weight = 1.f - total;	
			}
			xformAddVariation(xform, var, weight);
			plog(LOG_INFO, "\tnew variation: %d, weight %f\n", var, weight);
		}
		free(varsDone);
	}


}

static void flameSetXformColours(Flame *flame) {
	for (int i = 0; i < flame->nXforms; i++) {
		Xform *xform = &flame->xforms[i];
		paletteGetColour(flame->palette, xform->colourIndex, &xform->colour);
	}
}
