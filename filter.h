#ifndef FILTER_H
#define FILTER_H

#include "flame.h"

struct FlameStruct;
//typedef struct FlameStruct Flame;

typedef struct {
	float minRadius;
	float maxRadius;
	float alpha;
	int nKernels;
	int rowSize;
	int halfRowSize;
	float **kernels;
} DensityEstimationFilter;

//DensityEstimationFilter *filterCreate(float minWidth, float maxWidth, float alpha, int ss);
void filterDensityEstimation(struct FlameStruct *flame);
void filterCreate(struct FlameStruct *flame);


#endif

