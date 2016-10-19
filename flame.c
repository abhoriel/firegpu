#include <stdio.h>
#include <stdlib.h>
#include "log.h"
#include "flame.h"

#define FLOAT float

typedef struct {
	int r, g, b;
} Colour;

typedef struct {
	FLOAT a, b, c, d, e, f;
} Coefficients;

typedef struct {
	Coefficients main;
	Coefficients final;
	Colour col;
} Xform;

typedef struct {
	char *name;
	FLOAT weight;
} Variations;

typedef struct {
	Xform *xforms;
	int nXforms;

	int w, int h;
	FLOAT *histogram;
} Flame;

Flame *flameInit(int w, int h) {
	Flame *flame = malloc(sizeof(Flame));
	flame->w = w;
	flame->h = h;
	return flame;
}

int flameDraw(int w, int h) {
	float *histogram = malloc(w * h * sizeof(float));
	if (histogram == NULL) {
		plog(LOG_ERROR, "drawFlame(): memory allocation failure\n");
		return -1;
	}

	free(histogram);
	return 0;
}
