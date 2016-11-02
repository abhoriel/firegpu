#include "filter.h"


float epanechnikovKernel(float u) {
	return 0.75f * (1.f - (u * u));
}
