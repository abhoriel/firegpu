// kernel for generating flame fractals
// this will be modified by the main program 

#define PREVENT_DIVIDE_BY_ZERO	0.0000000001f

typedef struct {
	unsigned int q;
	unsigned int c;
} Rng;

typedef struct __attribute__ ((packed)) {
	float r, g, b;
} Colour;

typedef struct __attribute__ ((packed)) {
	Colour c;
	float intensity;
} Pixel;

typedef struct __attribute__ ((packed)) {
	int w;
	int h;
	int supersample;
	int iterations;
	int seed;
} Flame;

typedef struct __attribute__ ((packed)) {
	Colour colour;
	float a, b, c, d, e, f;
	float opacity;
} Xform;


unsigned int rngGenerate32(Rng *rng);
float rngGenerateFloat(Rng *rng, float min, float max);
inline void atomicAdd(volatile __global float *addr, float val);

unsigned int rngGenerate32(Rng *rng) {
	unsigned long a = 809430660L;
	unsigned long t;
	t = a * rng->q + rng->c; 
	rng->c = (t >> 32);
	rng->q = t;
	return rng->q;
}

float rngGenerateFloat(Rng *rng, float min, float max) {
	float diff = max - min;
	return min + ((float)rngGenerate32(rng) / (4294967295.f / diff));
}

__kernel void generate(__constant Flame *flame, __constant Xform *xforms, __constant int *xfd, __global Pixel *pixels) {
	Rng rng;
	rng.q = flame->seed;
	rng.c = 362436;
	// this way each work item should get a different seed
	rng.q += get_global_id(0);

	float x = rngGenerateFloat(&rng, -1.f, 1.f);
	float y = rngGenerateFloat(&rng, -1.f, 1.f);

	for (int j = -20; j < flame->iterations; j++) {
		// decide which xform to use
		int xformIndex = xfd[rngGenerate32(&rng) & (XFORM_DISTRIBUTION_SIZE - 1)];

		float ox, oy;

		//printf("a %f, b %f, c %f, opacity %f, r %f, g %f, g %f\n", xforms[xformIndex].a, xforms[xformIndex].b, xforms[xformIndex].c, xforms[xformIndex].opacity,  xforms[xformIndex].colour.r, xforms[xformIndex].colour.g, xforms[xformIndex].colour.b);
		ox = x * xforms[xformIndex].a + y * xforms[xformIndex].b + xforms[xformIndex].c;
		oy = x * xforms[xformIndex].d + y * xforms[xformIndex].e + xforms[xformIndex].f;

		//printf("%d %d %d %d\n", flame->w, flame->h, flame->supersample, flame->iterations);

// APPLY_VARIATIONS

// APPLY_POST_TRANSFORM

// APPLY_FINAL_TRANSFORM

		// after the first 20 iterations, we plot the points
		if (j > 0) {
			int xi = (x + 1.f) * 0.5f * flame->w * flame->supersample;
			int yi = (y + 1.f) * 0.5f * flame->h * flame->supersample;
			if ((xi < 0) || (xi >= (flame->w * flame->supersample)) || (yi < 0) || (yi >= (flame->h * flame->supersample))) {
				continue;
			}
			__global Pixel *pixel = &pixels[xi + (yi * flame->w * flame->supersample)];
			float opacity = xforms[xformIndex].opacity;
			
			/*
			pixel->c.r += xforms[xformIndex].colour.r * opacity;
			pixel->c.g += xforms[xformIndex].colour.g * opacity;
			pixel->c.b += xforms[xformIndex].colour.b * opacity;
			pixel->intensity += opacity; 
			*/
			
			atomicAdd(&pixel->c.r, xforms[xformIndex].colour.r * opacity);
			atomicAdd(&pixel->c.g, xforms[xformIndex].colour.g * opacity);
			atomicAdd(&pixel->c.b, xforms[xformIndex].colour.b * opacity);
			atomicAdd(&pixel->intensity, opacity);
		}
	}
	return;
}

// this elegant atomic add routine is from:
// https://streamcomputing.eu/blog/2016-02-09/atomic-operations-for-floats-in-opencl-improved/
inline void atomicAdd(volatile __global float *addr, float val) {
	union {
		unsigned int u32;
		float f32;
	} next, expected, current;

	current.f32 = *addr;
	do {
		expected.f32 = current.f32;
		next.f32 = expected.f32 + val;
		current.u32 = atomic_cmpxchg((volatile __global unsigned int *)addr, expected.u32, next.u32);
	} while(current.u32 != expected.u32);
}

