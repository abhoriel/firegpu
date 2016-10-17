#include <stdio.h>
#include <stdlib.h>
#include "log.h"
#include "flame.h"

typedef struct {


} Flame;


typedef struct {
	void (*fn)(float *, float *);	// too slow i think

} Variation;

int drawFlame(int w, int h) {
	float *histogram = malloc(w * h * sizeof(float));
	if (histogram == NULL) {
		plog(LOG_ERROR, "drawFlame(): memory allocation failure\n");
		return -1;
	}

	free(histogram);
	return 0;
}
