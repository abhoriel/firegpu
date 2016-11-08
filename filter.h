#ifndef FILTER_H
#define FILTER_H

typedef struct {
	float minWidth;
	float maxWidth;
	float alpha;
	int nKernels;
	float **kernels;
} DensityEstimationFilter;

#endif

