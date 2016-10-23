#include <stdio.h>
#include <stdlib.h>
#include "log.h"
#include "xform.h"
#include "rng.h"
#include "variation.h"
#include "flame.h"

// MUST be a power of 2
#define XFORM_DISTRIBUTION_SIZE	4096

typedef struct {
	float r, g, b;
} Colour;

typedef struct {
	Colour *colours;
	int nColours;
} Palette;

typedef struct {
	char *name;
	FLOAT weight;
} Variations;


static int *createXformDistribution(Flame *flame);


Flame *flameCreate() {
	Flame *flame = malloc(sizeof(Flame));
	flame->xforms = NULL;
	flame->nXforms = 0;
	flame->histogram = NULL;
	return flame;
}

void flameDestroy(Flame *flame) {
	if (flame->histogram != NULL) {
		free(flame->histogram);
	}
	free(flame);
}

Xform *flameCreateXform(Flame *flame) {
	Xform *xform = xformCreate();
	if (xform == NULL) {
		return NULL;
	}
	flame->xforms = realloc(flame->xforms, sizeof(Xform) * (flame->nXforms + 1));
	if (flame->xforms == NULL) {
		plog(LOG_ERROR, "flameCreateXform(): memory allocation failed\n");
		xformDestroy(xform);
		return NULL;
	}
	flame->nXforms++;
	return xform;
}


int flameGenerate(Flame *flame) {
	int nSamples = flame->w * flame->superSample * flame->h * flame->superSample;
	float *histogram = malloc(nSamples * sizeof(float));
	Colour *colours = malloc(nSamples * sizeof(Colour));
	if ((histogram == NULL) || (colours == NULL)){
		plog(LOG_ERROR, "drawFlame(): memory allocation failure\n");
		return -1;
	}
	for (int i = 0; i < nSamples; i++) {
		histogram[i] = 0.f;
		colours[i].r = 0.f;
		colours[i].g = 0.f;
		colours[i].b = 0.f;
	}


	int *xfd = createXformDistribution(flame);
	int quality = flame->quality * nSamples;
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
				int xi = (x + 1.f) * 0.5 * flame->w * flame->superSample;
				int yi = (y + 1.f) * 0.5 * flame->h * flame->superSample;
				Colour *colour = &colours[xi + (yi * flame->w * flame->superSample)];
				if (histogram[xi + (yi * flame->w * flame->superSample)] == 0.f) {
					//colour->r = xform->
				}
				histogram[xi + (yi * flame->w * flame->superSample)] += xform->opacity; 
			}
		}
	}

	if (flame->histogram != NULL) {
		free(flame->histogram);
	}
	flame->histogram = histogram;
	return 0;
}

static int *createXformDistribution(Flame *flame) {
	int *xfd = malloc(sizeof(int) * XFORM_DISTRIBUTION_SIZE);
	int xform = 0;
	float offset = 0;
	for (int i = 0; i < XFORM_DISTRIBUTION_SIZE; i++) {
		float p = ((float)i / XFORM_DISTRIBUTION_SIZE);
		if (((p - offset) > flame->xforms[xform].weight) && (xform < (flame->nXforms - 1))) {
			xform++;
			offset += p;
		}
		xfd[i] = xform;
	}
	return xfd;
}

