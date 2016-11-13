#include <SDL2/SDL.h>
#include <time.h>
#include <errno.h>

#include "log.h"
#include "flame.h"
#include "xform.h"
#include "palette.h"
#include "opencl.h"
#include "filter.h"
#include "source.h"
#include "sdl.h"

static void initFps();
static float calculateFps();
static int buildSource(Flame *flame);
static int drawFractal(Flame *flame, int w, int h, int densityEstimation);

static SDL_Window *window = NULL;
static SDL_Renderer *renderer = NULL;
static SDL_Texture * texture;
static Uint32 *pixels;
static int width, height;

int sdlInit(int w, int h) {
	if(SDL_Init(SDL_INIT_VIDEO) < 0) {
		plog(LOG_ERROR, "could not initialise sdl: %s\n", SDL_GetError());
		return -1;
	}
	
	window = SDL_CreateWindow("firegpu", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, w, h, SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);
	if(window == NULL) {
		plog(LOG_ERROR, "sdl window could not be created: %s\n", SDL_GetError());
		return -1;
	}
	
	width = w;
	height = h;

	renderer = SDL_CreateRenderer(window, -1, 0);
	if (renderer == NULL) {
		plog(LOG_ERROR, "sdl renderer could not be created: %s\n", SDL_GetError());
		return -1;
	}
	
	texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STATIC, w, h);
	if (renderer == NULL) {
		plog(LOG_ERROR, "sdl renderer could not be created: %s\n", SDL_GetError());
		return -1;
	}

	pixels = (Uint32 *)malloc(sizeof(Uint32) * w * h);
	if (pixels == NULL) {
		plog(LOG_ERROR, "memory allocation failed: %s\n", strerror(errno));
		return -1;
	}
	memset(pixels, 255, sizeof(Uint32) * w * h);
		
	return 0;
}

void sdlFini() {
	free(pixels);
	SDL_DestroyTexture(texture);
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_Quit();
}

void sdlMain() {
	typedef enum {NORMAL, DE} Mode;

	Mode mode = NORMAL;
	int quit = 0;
	SDL_Event event;

	initFps();
	
	int leftMouseButtonDown = 0;
	int rightMouseButtonDown = 0;
	int mouseX = 0;
	int mouseY = 0;
	int mustRecreateSamplesBuffer = 0;
	int mustRecreateTexture = 0;
	int mustRecreateDensityEstimationFilter = 0;
	int mustRebuildSource = 0;
	int densityEstimation = 0;

	Flame *flame = flameCreate();
	flame->supersample = 1;
	flame->w = width;
	flame->h = height;
	flame->iterations = 5; //?
	flame->quality = 5;	// should probably be increased to a few thousand
	flame->gamma = 2.0f;
	paletteAddColour(flame->palette, 0.1f, 0.1f, 1.f);
	paletteAddColour(flame->palette, 1.f, 0.1f, 1.f);
	paletteAddColour(flame->palette, 1.f, 0.1f, 0.1f);
	paletteAddColour(flame->palette, 1.f, 0.75f, 0.1f);
	paletteAddColour(flame->palette, 1.f, 1.f, 1.f);

	Xform *xform = flameCreateXform(flame);
	xform->hasPost = 0;
	xform->weight = 0.951f;
	xform->colourIndex = 0.0f;
	xform->opacity = 1.0f;
	xform->symmetry = 0.0f;
	xform->coMain.a = 1.0743f; 	
	xform->coMain.b = 0.276938f;
	xform->coMain.c = -0.229114f;
	xform->coMain.d = -1.13321f;
	xform->coMain.e = 1.31898;
	xform->coMain.f = -0.07108f;
	xformAddVariation(xform, 2, 1.0f);	// spherical

	Xform *xform2 = flameCreateXform(flame);
	xform2->hasPost = 0;
	xform2->weight = 0.007;
	xform2->colourIndex = 0.5f;
	xform2->opacity = 1.0f;
	xform2->symmetry = 0.0f;
	xform2->coMain.a = -0.08903f; 
	xform2->coMain.b = -0.668048;
	xform2->coMain.c = 0.624383;
	xform2->coMain.d = -0.086163;
	xform2->coMain.e = -0.759701;
	xform2->coMain.f = 0.265096;
	xformAddVariation(xform2, 2, 1.0f);	// spherical

	int rightleftPressed = 0;
	int updownPressed = 0;

	filterCreate(flame);

	if (buildSource(flame) < 0) {
		return;
	}

	while (!quit) {
		int ret = drawFractal(flame, width, height, densityEstimation);
		if (ret < 0) {
			quit = 1;
		}

		// display the texture on the screen
		SDL_UpdateTexture(texture, NULL, pixels, width * sizeof(Uint32));
		SDL_RenderCopy(renderer, texture, NULL, NULL);
		SDL_RenderPresent(renderer);
	
		const Uint8 *state = SDL_GetKeyboardState(NULL);
		if (state[SDL_SCANCODE_D]) {
			mode = DE;
		} else {
			mode = NORMAL;
		}

		while(SDL_PollEvent(&event)) {
			switch (event.type) {
				case SDL_QUIT:
					quit = 1;
					break;
				case SDL_MOUSEBUTTONUP:
					if (event.button.button == SDL_BUTTON_LEFT) {
						leftMouseButtonDown = 0;
					}
					if (event.button.button == SDL_BUTTON_RIGHT) {
						rightMouseButtonDown = 0;
					}
					break;
				case SDL_MOUSEBUTTONDOWN:
					if (event.button.button == SDL_BUTTON_LEFT) {
						leftMouseButtonDown = 1;
					}
					if (event.button.button == SDL_BUTTON_RIGHT) {
						rightMouseButtonDown = 1;
					}
					break;
				case SDL_MOUSEMOTION:
					mouseX = event.motion.x;
					mouseY = event.motion.y;
					break;
				case SDL_KEYDOWN:
					switch (event.key.keysym.sym) {
						case SDLK_ESCAPE:
							quit = 1;
							break;
						case SDLK_RETURN:
							flameRandomise(flame);
							mustRebuildSource = 1;
							break;
						case SDLK_SPACE:
							if (mode == DE) {
								if (densityEstimation) {
									densityEstimation = 0;
								} else {
									densityEstimation = 1;
								}
							}
							break;
						case SDLK_1:
							flame->supersample = 1;
							mustRecreateSamplesBuffer = 1;
							break;
						case SDLK_2:
							flame->supersample = 2;
							mustRecreateSamplesBuffer = 1;
							break;
						case SDLK_3:
							flame->supersample = 3;
							mustRecreateSamplesBuffer = 1;
							break;
						case SDLK_4:
							flame->supersample = 4;
							mustRecreateSamplesBuffer = 1;
							break;
						case SDLK_5:
							flame->supersample = 5;
							mustRecreateSamplesBuffer = 1;
							break;
						case SDLK_6:
							flame->supersample = 6;
							mustRecreateSamplesBuffer = 1;
							break;
						case SDLK_7:
							flame->supersample = 7;
							mustRecreateSamplesBuffer = 1;
							break;
						case SDLK_8:
							flame->supersample = 8;
							mustRecreateSamplesBuffer = 1;
							break;
						case SDLK_r:
							break;
						case SDLK_UP:
							updownPressed = 1;
						break;
						case SDLK_DOWN:
							updownPressed = -1;
							break;
						case SDLK_RIGHT:
							rightleftPressed = 1;
							/*
							flame->iterations *= 5;
							plog(LOG_INFO, "iterations: %d\n", flame->iterations);
							*/
							break;
						case SDLK_LEFT:
							rightleftPressed = -1;
							break;
						case SDLK_q:
							flame->gamma += 0.25f;
							plog(LOG_INFO, "gamma: %f\n", flame->gamma);
							break;
						case SDLK_a:
							flame->gamma -= 0.25f;
							plog(LOG_INFO, "gamma: %f\n", flame->gamma);
						case SDLK_n:
							break;
					}
			}
			if (event.type == SDL_WINDOWEVENT) {
				switch (event.window.event) {
					case SDL_WINDOWEVENT_RESIZED:
						width = event.window.data1;
						height = event.window.data2;
						flame->w = width;
						flame->h = height;
						mustRecreateSamplesBuffer = 1;
						mustRecreateTexture = 1;
						break;
				}
			}
		}

		if (updownPressed != 0) {
			switch(mode) {
				case DE:
					if (updownPressed == 1) {
						flame->def.alpha += 0.1;
					} else {
						flame->def.alpha -= 0.1;	
					}
					plog(LOG_INFO, "curve alpha: %f\n", flame->def.alpha);
					mustRecreateDensityEstimationFilter = 1;
					break;
				case NORMAL:
				default:
					if (updownPressed == 1) {
						flame->quality *= 5;
					} else {
						flame->quality /= 5;
					}
					plog(LOG_INFO, "quality: %d\n", flame->quality);
						break;
			}
			updownPressed = 0;
		}
		if (rightleftPressed != 0) {
			switch(mode) {
				case DE:
					flame->def.maxRadius += rightleftPressed;	
					plog(LOG_INFO, "max kernel radius: %f\n", flame->def.maxRadius);
					mustRecreateDensityEstimationFilter = 1;
					break;
				case NORMAL:
				default:
					if (rightleftPressed == 1) {
						flame->iterations *= 5;
					} else {
						flame->iterations /= 5;
					}
					plog(LOG_INFO, "iterations: %d\n", flame->iterations);
					break;
			}
			rightleftPressed = 0;
		}


		if (mustRebuildSource) {
			openclFiniProgram();
			if (buildSource(flame) < 0) {
				return;
			}
			mustRebuildSource = 0;
		}	
		if (mustRecreateDensityEstimationFilter) {
			filterCreate(flame);
			mustRecreateDensityEstimationFilter = 0;
		}
		if (mustRecreateSamplesBuffer) {
			mustRecreateSamplesBuffer = 0;
		}
		if (mustRecreateTexture) {
			SDL_DestroyTexture(texture);
			texture = SDL_CreateTexture(renderer,
				SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STATIC, width, height);
			free(pixels);
			pixels = (Uint32 *)malloc(sizeof(Uint32) * width * height);
			mustRecreateTexture = 0;
		}

		
		if (leftMouseButtonDown || rightMouseButtonDown) {
			(void)mouseX; (void)mouseY;
			/*
			centreX += ((mouseX - width / 2) * scale) * 0.02;
			centreY += ((mouseY - height / 2) * scale) * 0.02;
			if (leftMouseButtonDown) {
				scale *= 0.99;
			}
			if (rightMouseButtonDown) {
				scale /= 0.99;
			}
			*/
		}
		
		//topLeftX = centreX - ((width * scale) / 2);
		//topLeftY = centreY - ((height * scale) / 2);
		
		char title[64];
		sprintf(title, "firegpu (fps: %.1f)", calculateFps());
		SDL_SetWindowTitle(window, title);
	}
	
	openclFiniProgram();
	flameDestroy(flame);
}


static int drawFractal(Flame *flame, int w, int h, int densityEstimation) {
	// generate the flame, tonemap and downsample
	int ret;

	clock_t time0 = clock();
	ret = flameGenerate(flame);
	if (ret < 0) {
		return ret;
	}

	clock_t time1 = clock();
	if (densityEstimation) {
		filterDensityEstimation(flame);
	}

	clock_t time2 = clock();
	flameTonemap(flame);
	if (flame->supersample > 1) {
		flameDownsample(flame);
	}
	clock_t time3 = clock();
	float genTime = ((float)time1 - time0) / CLOCKS_PER_SEC;
	float deTime = ((float)time2 - time1) / CLOCKS_PER_SEC;

	float tmTime = ((float)time3 - time2) / CLOCKS_PER_SEC;
	plog(LOG_INFO, "done. time taken: generation: %.2fs, density estimation %.2fs, tonemapping %.2fs\n", genTime, deTime, tmTime);

	// write the flame into the texture
	for (int y = 0; y < h; y++) {
		for (int x = 0; x < w; x++) {
			Pixel *pixel = &flame->pixels[(y * flame->w * flame->supersample) + x];
			((Uint8 *)pixels)[(w * y * 4) + (x * 4) + 0] = (Uint8)(pixel->c.b * 255);
			((Uint8 *)pixels)[(w * y * 4) + (x * 4) + 1] = (Uint8)(pixel->c.g * 255);
			((Uint8 *)pixels)[(w * y * 4) + (x * 4) + 2] = (Uint8)(pixel->c.r * 255);
			((Uint8 *)pixels)[(w * y * 4) + (x * 4) + 3] = (Uint8)(255);
		}
	}
	return 0;
}

static int buildSource(Flame *flame) {
	Source *src = flameGenerateSource(flame);
	plog(LOG_VERBOSE, "%s\n", src->buffer);
	int ret = openclBuildProgram(src->buffer);
	sourceDestroy(src);
	return ret;
}


#if 0
void sdlMain() {
	int quit = 0;
	SDL_Event event;
	static float startingScale = 0.010;
	
	// h has to be a multiple of 64 currently..
	double scale = startingScale;
	int nSamples = 1; //scale /= nSamples;
	
	//double centreX = 0.435396403; double centreY = 0.367981352;
	double centreX = 0; double centreY = 0;
	double topLeftX = centreX - ((width * scale) / 2);
	double topLeftY = centreY - ((height * scale) / 2);
	int limit = 512;

	double *samples = (double *)malloc(sizeof(double) * width * height * nSamples * nSamples);
	if (samples == NULL) {
		plog(LOG_ERROR, "memory allocation failed: %s\n", strerror(errno));
		return;
	}

	initFps();
	
	int leftMouseButtonDown = 0;
	int rightMouseButtonDown = 0;
	int mouseX = 0;
	int mouseY = 0;
	int mustRecreateSamplesBuffer = 0;
	int mustRecreateTexture = 0;
	int normalise = 1;
	
	while (!quit) {
		int ret = openclExecMandlebrot(width * nSamples, height * nSamples, scale / nSamples, topLeftX, topLeftY, limit, samples);
		if (ret != 0) {
			plog(LOG_ERROR, "error executing opencl kernel\n");
			return;
		}

		drawFractal(samples, width, height, nSamples, normalise);
		SDL_UpdateTexture(texture, NULL, pixels, width * sizeof(Uint32));
		//SDL_RenderClear(renderer);
		SDL_RenderCopy(renderer, texture, NULL, NULL);
		SDL_RenderPresent(renderer);
		
		while(SDL_PollEvent(&event)) {
			switch (event.type) {
				case SDL_QUIT:
					quit = 1;
					break;
				case SDL_MOUSEBUTTONUP:
					if (event.button.button == SDL_BUTTON_LEFT)
						leftMouseButtonDown = 0;
					if (event.button.button == SDL_BUTTON_RIGHT)
						rightMouseButtonDown = 0;
					break;
				case SDL_MOUSEBUTTONDOWN:
					if (event.button.button == SDL_BUTTON_LEFT)
						leftMouseButtonDown = 1;
					if (event.button.button == SDL_BUTTON_RIGHT)
						rightMouseButtonDown = 1;
					break;
				case SDL_MOUSEMOTION:
					mouseX = event.motion.x;
					mouseY = event.motion.y;
					break;
				case SDL_KEYDOWN:
					switch (event.key.keysym.sym) {
						case SDLK_ESCAPE:
							quit = 1;
							break;
						case SDLK_1:
							nSamples = 1;
							mustRecreateSamplesBuffer = 1;
							break;
						case SDLK_2:
							nSamples = 2;
							mustRecreateSamplesBuffer = 1;
							break;
						case SDLK_3:
							nSamples = 3;
							mustRecreateSamplesBuffer = 1;
							break;
						case SDLK_4:
							nSamples = 4;
							mustRecreateSamplesBuffer = 1;
							break;
						case SDLK_5:
							nSamples = 5;
							mustRecreateSamplesBuffer = 1;
							break;
						case SDLK_6:
							nSamples = 6;
							mustRecreateSamplesBuffer = 1;
							break;
						case SDLK_7:
							nSamples = 7;
							mustRecreateSamplesBuffer = 1;
							break;
						case SDLK_8:
							nSamples = 8;
							mustRecreateSamplesBuffer = 1;
							break;
						case SDLK_r:
							scale = 0.004;
							nSamples = 1;
							limit = 512;
							centreX = 0; centreY = 0;
							break;
						case SDLK_UP:
							if (limit < 1000000000) {
								limit *= 1.1;
							}
							plog(LOG_INFO, "limit: %d\n", limit);
							break;
						case SDLK_DOWN:
							if (limit > 10) {
								limit /= 1.1;
							}
							plog(LOG_INFO, "limit: %d\n", limit);
							break;
						case SDLK_RIGHT:
							break;
						case SDLK_LEFT:
							break;
						case SDLK_n:
							if (normalise) {
								normalise = 0;
								plog(LOG_INFO, "normalisation off\n");
							} else {
								normalise = 1;
								plog(LOG_INFO, "normalisation on\n");
							}
							break;
					}
			}
			if (event.type == SDL_WINDOWEVENT) {
				switch (event.window.event) {
					case SDL_WINDOWEVENT_RESIZED:
						width = event.window.data1;
						height = event.window.data2;
						mustRecreateSamplesBuffer = 1;
						mustRecreateTexture = 1;
						break;
				}
			}
		}
		
		if (mustRecreateSamplesBuffer) {
			free(samples);
			samples = (double *)malloc(sizeof(double) * width * height * nSamples * nSamples);
			mustRecreateSamplesBuffer = 0;
		}
		if (mustRecreateTexture) {
			SDL_DestroyTexture(texture);
			texture = SDL_CreateTexture(renderer,
				SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STATIC, width, height);
			free(pixels);
			pixels = (Uint32 *)malloc(sizeof(Uint32) * width * height);
			mustRecreateTexture = 0;
		}
		
		if (leftMouseButtonDown || rightMouseButtonDown) {
			centreX += ((mouseX - width / 2) * scale) * 0.02;
			centreY += ((mouseY - height / 2) * scale) * 0.02;
			if (leftMouseButtonDown) {
				scale *= 0.99;
			}
			if (rightMouseButtonDown) {
				scale /= 0.99;
			}
		}
		
		topLeftX = centreX - ((width * scale) / 2);
		topLeftY = centreY - ((height * scale) / 2);
		
		char title[64];
		sprintf(title, "firegpu (fps: %.1f)", calculateFps());
		SDL_SetWindowTitle(window, title);
		//printf("fps %f\n", calculateFps());
	}
	
	free(samples);
}


static void drawFractal(double *samples, int w, int h, int nSamples, int normalise) {
	double *downSampled;
	if (nSamples > 1) {
		downSampled = downSample(samples, w, h, nSamples);
	} else {
		downSampled = samples;
	}
	
	// we must first get the minimum value in order to normalise
	// the colours.
	double min = 1;
	if (normalise) {
		for (int y = 0; y < h; ++y) {
			for (int x = 0; x < w; ++x) {
				if ((downSampled[(w * y) + x] < min))
					min = downSampled[(w * y) + x];
			}
		}
	}
	for (int y = 0; y < h; ++y) {
		for (int x = 0; x < w; ++x) {
			// normalise the colours
			double iter = downSampled[(w * y) + x];
			if (min < 1) {
				iter = (iter - min) * (1.0 / (1.0 - min));
			}
			
			((Uint8 *)pixels)[(w * y * 4) + (x * 4) + 0] = (Uint8)(iter * 255);
			((Uint8 *)pixels)[(w * y * 4) + (x * 4) + 1] = (Uint8)(iter * 255);
			((Uint8 *)pixels)[(w * y * 4) + (x * 4) + 2] = (Uint8)(iter * 255);
			((Uint8 *)pixels)[(w * y * 4) + (x * 4) + 3] = (Uint8)(iter * 255);
			//Colour col = gradient.getColour(iter);
			//pixels[(w * y) + (x)] = col.getargb();
		}
	}
	
	if (nSamples > 1) {
		free(downSampled);
	}
}



/* naive (mean average) downsample the fractal */
static double *downSample(double *samples, int w, int h, int nSamples) {
	double *downSampled = (double *)malloc(sizeof(double) * w * h);
	double total;
	for (int y = 0; y < h; ++y) {
		for (int x = 0; x < w; ++x) {
			total = 0;
			for (int osY = y * nSamples; osY < (y * nSamples) + nSamples; osY++) {
				for (int osX = x * nSamples; osX < (x * nSamples) + nSamples; osX++) {
					total += samples[(osY * w * nSamples) + osX];
				}
			}
			downSampled[(w * y) + x] = total / (nSamples * nSamples);
		}
	}
	return downSampled;
}
#endif

// This FPS code is from the SDL wiki

// How many frames time values to keep
// The higher the value the smoother the result is...
// Don't make it 0 or less :)
#define FRAME_VALUES 10

// An array to store frame times:
static Uint32 frametimes[FRAME_VALUES];

// Last calculated SDL_GetTicks
static Uint32 frametimelast;

// total frames rendered
static Uint32 framecount;

// This function gets called once on startup.
static void initFps() {
		// Set all frame times to 0ms.
		memset(frametimes, 0, sizeof(frametimes));
		framecount = 0;
		frametimelast = SDL_GetTicks();
}

static float calculateFps() {
	Uint32 frametimesindex;
	Uint32 getticks;
	Uint32 count;
	Uint32 i;
	float framesPerSecond;

	// frametimesindex is the position in the array. It ranges from 0 to FRAME_VALUES.
	// This value rotates back to 0 after it hits FRAME_VALUES.
	frametimesindex = framecount % FRAME_VALUES;

	// store the current time
	getticks = SDL_GetTicks();

	// save the frame time value
	frametimes[frametimesindex] = getticks - frametimelast;

	// save the last frame time for the next fpsthink
	frametimelast = getticks;

	// increment the frame count
	framecount++;

	// Work out the current framerate

	// The code below could be moved into another function if you don't need the value every frame.

	// I've included a test to see if the whole array has been written to or not. This will stop
	// strange values on the first few (FRAME_VALUES) frames.
	if (framecount < FRAME_VALUES) {
		count = framecount;
	} else {
		count = FRAME_VALUES;

	}

	// add up all the values and divide to get the average frame time.
	framesPerSecond = 0;
	for (i = 0; i < count; i++) {
		framesPerSecond += frametimes[i];
	}

	framesPerSecond /= count;

	// now to make it an actual frames per second value...
	framesPerSecond = 1000.f / framesPerSecond;

	return framesPerSecond;
		
}
