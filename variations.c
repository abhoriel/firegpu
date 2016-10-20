#include <math.h>
#include "firegpu.h"
#include "xform.h"
#include "rng.h"
#include "variations.h"

typedef struct {
	char *name;
	void (*fn)(Xform *, FLOAT *, FLOAT *);
} Variation;

static void linear(Xform *xform, FLOAT *x, FLOAT *y);
static void sinusoidal(Xform *xform, FLOAT *x, FLOAT *y);
static void spherical(Xform *xform, FLOAT *xp, FLOAT *yp);
static void swirl(Xform *xform, FLOAT *xp, FLOAT *yp);
static void horseshoe(Xform *xform, FLOAT *xp, FLOAT *yp);
static void polar(Xform *xform, FLOAT *xp, FLOAT *yp);
static void handkerchief(Xform *xform, FLOAT *xp, FLOAT *yp);
static void heart(Xform *xform, FLOAT *xp, FLOAT *yp);
static void disc(Xform *xform, FLOAT *xp, FLOAT *yp);
static void spiral(Xform *xform, FLOAT *xp, FLOAT *yp);

Variation variations[] = {
	{"linear", 		linear},
	{"sinusoidal",		sinusoidal},
	{"spherical",		spherical},
	{"swirl",			swirl},
	{"horseshoe",		horseshoe},
	{"polar",			polar},
	{"handkerchief",	handkerchief},
	{"heart",			heart},
	{"disc",			disc},
	{"spiral",			spiral}

};


void variationInit() {
	
}

static void linear(Xform *xform, FLOAT *x, FLOAT *y) {
	(void)xform;
	(void)x;
	(void)y;
}

static void sinusoidal(Xform *xform, FLOAT *x, FLOAT *y) {
	(void)xform;
	*x = sinf(*x);
	*y = sinf(*y);
}

static void spherical(Xform *xform, FLOAT *xp, FLOAT *yp) {
	(void)xform;
	FLOAT x = *xp;
	FLOAT y = *yp;
	FLOAT r = sqrtf((x * x) + (y * y));
	*xp = x / (r * r);
	*yp = y / (r * r);
}

static void swirl(Xform *xform, FLOAT *xp, FLOAT *yp) {
	(void)xform;
	FLOAT x = *xp;
	FLOAT y = *yp;
	FLOAT r = sqrtf((x * x) + (y * y));
	FLOAT theta = atanf(x / y);
	*xp = r * cosf(theta + r);
	*yp = r * sinf(theta + r);
}

static void horseshoe(Xform *xform, FLOAT *xp, FLOAT *yp) {
	(void)xform;
	FLOAT x = *xp;
	FLOAT y = *yp;
	FLOAT r = sqrtf((x * x) + (y * y));
	FLOAT theta = atanf(x / y);
	*xp = r * cosf(theta * 2);
	*yp = r * sinf(theta * 2);
}

static void polar(Xform *xform, FLOAT *xp, FLOAT *yp) {
	(void)xform;
	FLOAT x = *xp;
	FLOAT y = *yp;
	FLOAT r = sqrtf((x * x) + (y * y));
	FLOAT theta = atanf(x / y);
	//FLOAT omega = ((FLOAT)(rngGenerate32() & 1)) * M_PI;
	*xp = theta / M_PI;
	*yp = r - 1.;
}

static void handkerchief(Xform *xform, FLOAT *xp, FLOAT *yp) {
	(void)xform;
	FLOAT x = *xp;
	FLOAT y = *yp;
	FLOAT r = sqrtf((x * x) + (y * y));
	FLOAT theta = atanf(x / y);
	*xp = r * sinf(theta + r);
	*yp = r * cosf(theta - r);
}

static void heart(Xform *xform, FLOAT *xp, FLOAT *yp) {
	(void)xform;
	FLOAT x = *xp;
	FLOAT y = *yp;
	FLOAT r = sqrtf((x * x) + (y * y));
	FLOAT theta = atanf(x / y);
	*xp = r * sinf(theta * r);
	*yp = -r * cosf(theta * r);
}

static void disc(Xform *xform, FLOAT *xp, FLOAT *yp) {
	(void)xform;
	FLOAT x = *xp;
	FLOAT y = *yp;
	FLOAT r = sqrtf((x * x) + (y * y));
	FLOAT theta = atanf(x / y);
	*xp = theta * sinf(M_PI * r) / M_PI;
	*yp = theta * cosf(M_PI * r) / M_PI;
}

static void spiral(Xform *xform, FLOAT *xp, FLOAT *yp) {
	(void)xform;
	FLOAT x = *xp;
	FLOAT y = *yp;
	FLOAT r = sqrtf((x * x) + (y * y));
	FLOAT theta = atanf(x / y);
	*xp = (cosf(theta) + sinf(r)) / r;
	*yp = (sinf(theta) - cosf(r)) / r;
}

static void hyperbolic(Xform *xform, FLOAT *xp, FLOAT *yp) {
	(void)xform;
	FLOAT x = *xp;
	FLOAT y = *yp;
	FLOAT r = sqrtf((x * x) + (y * y));
	FLOAT theta = atanf(x / y);
	*xp = sinf(theta) / r;
	*yp = cosf(theta) / r;
}
