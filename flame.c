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
	Colour col;
} Affine;

typedef struct {
	char *name;
	FLOAT weight;
} Variations;

typedef struct {
	Affine *affines;
	int nAffines;

	FLOAT *histogram;
	

} Flame;

Flame *flameInit() {
	Flame *flame = malloc(sizeof(Flame));
	

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
