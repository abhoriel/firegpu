#ifndef RNG_H
#define RNG_H

void rngSeed(unsigned long seed);
unsigned int rngGenerate32();

static inline float rngGenerateFloat(float min, float max) {
	float diff = max - min;
	return min + ((float)rngGenerate32() / (4294967295.f / diff));
}

#endif
