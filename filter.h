#ifndef FILTER_H
#define FILTER_H

typedef struct {
	float minWidth;
	float maxWidth;
	float alpha;
	int nKernels;
	float **kernels;
} DensityEstimationFilter;

DensityEstimationFilter *filterCreate(float minWidth, float maxWidth, float alpha);

#endif

