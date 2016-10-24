#include <stdlib.h>
#include <assert.h>
#include <math.h>
#include "palette.h"

Palette *paletteCreate() {
	Palette *palette = malloc(sizeof(Palette));
	palette->colours = NULL;
	palette->nColours = 0;
	return palette;
}

void paletteDestroy(Palette *palette) {
	if (palette->colours != NULL) {
		free(palette->colours);
	}
	free(palette);
}

void paletteAddColour(Palette *palette, float r, float g, float b) {
	palette->colours = realloc(palette->colours, sizeof(Colour) * (palette->nColours + 1));
	palette->colours[palette->nColours].r = r;
	palette->colours[palette->nColours].g = g;
	palette->colours[palette->nColours].b = b;
	palette->nColours++;
}


void paletteGetColour(Palette *palette, float d, Colour *colour) {
	assert(d >= 0.f);
	assert(d <= 1.f);
	float scaled = d * palette->nColours;
	int i1 = floorf(scaled);
	int i2 = ceilf(scaled);
	float delta = scaled - floorf(scaled);
	colour->r = palette->colours[i1].r + (palette->colours[i2].r - palette->colours[i1].r) * delta;
	colour->g = palette->colours[i1].g + (palette->colours[i2].g - palette->colours[i1].g) * delta;
	colour->b = palette->colours[i1].b + (palette->colours[i2].b - palette->colours[i1].b) * delta;
}
