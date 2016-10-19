
void rngSeed(unsigned long seed);
unsigned int rngGenerate32();

#include <stdio.h>
/*
static inline float rngGenerateFloat(float min, float max) {
	float diff = max - min;
	
	return min + ((float)rngGenerate32() / (4294967295.f / diff));
}
*/

static inline float rngGenerateFloat(float min, float max) {
	float diff = max - min;
	unsigned int r = rngGenerate32();
	printf("diff %f, r %d, div %.9g, sizeof(long): %lu\n", diff, r, (4294967295.f / diff), sizeof(long));

	return min + ((float)r / (4294967295.f / diff));
}
