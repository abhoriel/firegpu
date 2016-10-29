
typedef struct {
	float r, g, b;
} Colour;

typedef struct {
	Colour c;
	float intensity;
} Pixel;

typedef struct {
	unsigned int q;
	unsigned int c;
} Rng;

unsigned int rngGenerate32(Rng *rng);
float rngGenerateFloat(Rng *rng, float min, float max);

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

typedef struct {
	int quality;
	int iterations;
	int w, h;
} Flame;

__kernel void generate(__constant Flame *flame,  __constant int *xfd, __global Pixel *pixels) {

	Rng rng;
	rng.q = 123456789;	// this should be adjusted depending on the core
	rng.c = 362436;

	const int quality = flame->quality * flame->w * flame->h;
	for (int sample = 0; sample < quality; sample++) {
		float x = rngGenerateFloat(&rng, -1.f, 1.f);
		float y = rngGenerateFloat(&rng, -1.f, 1.f);

		(void)x;
		(void)y;
		for (int j = -20; j < flame->iterations; j++) {
			// decide which xform to use
			/*
			int xformIndex = xfd[rngGenerate32() & (XFORM_DISTRIBUTION_SIZE - 1)];
			Xform *xform = xforms[xformIndex];
			
			// in opencl we can use fma here
			float newx = x * xform->coMain.a + y * xform->coMain.b + xform->coMain.c;
			float newy = x * xform->coMain.d + y * xform->coMain.e + xform->coMain.f;
			
			//variationDo(xform, &newx, &newy);

			x = newx;
			y = newy;

			// after the first 20 iterations, we plot the points
			if (j > 0) {
				int xi = (x + 1.f) * 0.5f * flame->w * flame->supersample;
				int yi = (y + 1.f) * 0.5f * flame->h * flame->supersample;
				if ((xi < 0) || (xi >= (flame->w * flame->supersample)) || (yi < 0) || (yi >= (flame->h * flame->supersample))) {
					continue;
				}
				Pixel *pixel = &pixels[xi + (yi * flame->w * flame->supersample)];

				pixel->c.r += xform->colour.r * xform->opacity;
				pixel->c.g += xform->colour.g * xform->opacity;
				pixel->c.b += xform->colour.b * xform->opacity;

				pixel->intensity += xform->opacity; 
			}
			*/
		}
	}
	return;
}


