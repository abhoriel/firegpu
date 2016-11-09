#ifndef FILTER_H
#define FILTER_H

typedef struct {
	float minRadius;
	float maxRadius;
	float alpha;
	int nKernels;
	float **kernels;
} DensityEstimationFilter;

DensityEstimationFilter *filterCreate(float minWidth, float maxWidth, float alpha);

#endif

