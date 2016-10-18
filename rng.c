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

/* choose random initial c<809430660 and 256 random 32-bit integers for Q[] */
static unsigned long q = 123456789;
static unsigned long c = 362436;

void rngSeed(unsigned long seed) {
	q = seed;
}

unsigned long rngGenerate32() {
	unsigned long long a = 809430660LL;
	q = a * q + c;
	c = (q >> 32);
	return q; 
}

