typedef struct __attribute__ ((packed)) {
	float r, g, b;
} Colour;

typedef struct __attribute__ ((packed)) {
	Colour c;
	float intensity;
} Pixel;


