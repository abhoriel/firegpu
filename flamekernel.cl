
typedef struct {
	float r, g, b;
} Colour;

typedef struct {
	Colour c;
	float intensity;
} Pixel;

static unsigned long q = 123456789;
static unsigned int c = 362436;

void rngSeed(unsigned long seed) {
	q = seed;
}

unsigned int rngGenerate32() {
	unsigned long long a = 809430660LL;
	q = a * q + c;
	c = (q >> 32);
	return q; 
}

static inline float rngGenerateFloat(float min, float max) {
	float diff = max - min;
	return min + ((float)rngGenerate32() / (4294967295.f / diff));
}




//int flameGenerate(Flame *flame) {
//__kernel void fractal(int w, FLOAT scale, FLOAT topLeftX, FLOAT topLeftY, int limit, __global FLOAT *buffer) {
__kernel void generate(int w, FLOAT scale, FLOAT topLeftX, FLOAT topLeftY, int limit, __global Pixel *pixels) {

/*
	if (flame->pixels != NULL) {
		free(flame->pixels);
		flame->pixels = NULL;
	}

	int nSamples = flameGetNSamples(flame);
	Pixel *pixels = malloc(nSamples * sizeof(Pixel));
	if (pixels == NULL) {
		plog(LOG_ERROR, "drawFlame(): memory allocation failure\n");
		return -1;
	}
	for (int i = 0; i < nSamples; i++) {
		pixels[i].intensity = 0.f;
		pixels[i].c.r = 0.f;
		pixels[i].c.g = 0.f;
		pixels[i].c.b = 0.f;
	}

	int *xfd = createXformDistribution(flame);
*/

	const int quality = flame->quality * flame->w * flame->h;
	for (int sample = 0; sample < quality; sample++) {
		float x = rngGenerateFloat(-1.f, 1.f);
		float y = rngGenerateFloat(-1.f, 1.f);

		for (int j = -20; j < flame->iterations; j++) {
			// decide which xform to use
			int xformIndex = xfd[rngGenerate32() & (XFORM_DISTRIBUTION_SIZE - 1)];
			Xform *xform = &flame->xforms[xformIndex];
			
			// in opencl we can use fma here
			float newx = x * xform->coMain.a + y * xform->coMain.b + xform->coMain.c;
			float newy = x * xform->coMain.d + y * xform->coMain.e + xform->coMain.f;
			
			variationDo(xform, &newx, &newy);

			if (xform->hasFinal) {
				x = newx * xform->coFinal.a + newy + xform->coFinal.b + xform->coFinal.c;
				y = newx * xform->coFinal.d + newy + xform->coFinal.e + xform->coFinal.f;
				// there can be a variation here too..
			} else {
				x = newx;
				y = newy;
			}

			// after the first 20 iterations, we plot the points
			if (j > 0) {
				int xi = (x + 1.f) * 0.5f * flame->w * flame->supersample;
				int yi = (y + 1.f) * 0.5f * flame->h * flame->supersample;
				if ((xi < 0) || (xi >= (flame->w * flame->supersample)) || (yi < 0) || (yi >= (flame->h * flame->supersample))) {
					continue;
				}
				Pixel *pixel = &pixels[xi + (yi * flame->w * flame->supersample)];

				Colour temp;
				paletteGetColour(flame->palette, xform->colour, &temp);
				pixel->c.r += temp.r * xform->opacity;
				pixel->c.g += temp.g * xform->opacity;
				pixel->c.b += temp.b * xform->opacity;

				pixel->intensity += xform->opacity; 
			}
		}
	}
	plog(LOG_INFO, "\n");
	free(xfd);
	flame->pixels = pixels;
	return 0;
}


