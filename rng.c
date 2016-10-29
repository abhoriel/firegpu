#include <stdint.h>
#include "rng.h"

#if 0
// This is George Marsaglia's multiply with carry RNG algorithm
// choose random initial c<809430660 and 256 random 32-bit integers for Q[] 
static unsigned long Q[256],c=362436;  
unsigned long MWC256(void) {
	unsigned long long t,a=809430660LL;
	static unsigned char i=255;
	t=a*Q[++i]+c;
	c=(t>>32);
	return(Q[i]=t); 
}
#endif

/*
unsigned int rngGenerate32() {
	unsigned long long a = 809430660LL;
	q = a * q + c;
	c = (q >> 32);
	return q; 
}
*/

static uint32_t c = 362436;
static uint32_t q = 123456789;

uint32_t rngGenerate32() {
	uint64_t a = 809430660L;
	uint64_t t;
	t = a * q + c; 
	c = (t >> 32);
	q = t;
	return q;
}

void rngSeed(uint32_t seed) {
	q = seed;
}

