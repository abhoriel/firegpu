#ifndef PALETTE_H
#define PALETTE_H

typedef struct __attribute__ ((packed)) {
	float r, g, b;
} Colour;

typedef struct {
	Colour *colours;
	int nColours;
} Palette;

Palette *paletteCreate();
void paletteDestroy(Palette *palette);
void paletteAddColour(Palette *palette, float r, float g, float b);
void paletteGetColour(Palette *palette, float d, Colour *colour);

#endif
