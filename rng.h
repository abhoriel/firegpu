#ifndef RNG_H
#define RNG_H

#include <stdint.h>

void rngSeed(uint32_t seed);
uint32_t rngGenerate32();

static inline float rngGenerateFloat(float min, float max) {
	float diff = max - min;
	return min + ((float)rngGenerate32() / (4294967295.f / diff));
}

#endif
