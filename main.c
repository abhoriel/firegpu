#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <time.h>
#include "opencl.h"
#include "source.h"
#include "sdl.h"
#include "rng.h"
#include "log.h"

static void usage() {
	plog(LOG_ERROR, "Usage: firegpu\n");
}

static const char *opt_string = "t:?";

static const struct option long_opts[] = {
	{"help", no_argument, NULL, 'h'},
	{"verbose", no_argument, NULL, 'v'},
	{"platform", required_argument, NULL, 'p'},
	{"device", required_argument, NULL, 'd'},
	{NULL, no_argument, NULL, 0}
};

int verbose = 0;

int main(int argc, char **argv) {
	int w = 512, h = 512;
	//char fn[] = "flamekernel.cl";

	int desiredPlatform = 0;
	int desiredDevice = 0;
	int opt = 0;
	int long_index;
	opt = getopt_long(argc, argv, opt_string, long_opts, &long_index);
	while(opt != -1) {
		switch(opt) {
			case 'v':
				verbose = 1;
				break;
			case 'p':
				desiredPlatform = atoi(optarg);
				break;
			case 'd':
				desiredDevice = atoi(optarg);
				break;
			case 'h':
			case '?':
				usage();
				return EXIT_FAILURE;
				break;
			case 0:     /* long option without a short arg */
				//if(strcmp("randomize", long_opts[long_index].name) == 0) {
				//}
				break;
			default:
				// not reached
				break;
		}
		opt = getopt_long(argc, argv, opt_string, long_opts, &long_index);
	}

	long seed = time(NULL);
	plog(LOG_INFO, "rng seed: %ld\n", seed);
	rngSeed(seed);	

	if (sdlInit(w, h) != 0) {
		plog(LOG_ERROR, "error initialising SDL\n");
		return 1;
	}

	//(void)source; (void)desiredDevice; (void)desiredPlatform; (void)fn;
	
	if (openclInit(desiredPlatform, desiredDevice) != 0) {
		plog(LOG_ERROR, "error initialising openCL\n");
		return 1;
	}
	
	/*
	Source *source = sourceLoad(fn);
	if (source == NULL) {
		return 1;
	}

	//sourceReplace(source, "flame->w", "512");
	//sourceReplace(source, "flame->y", "512");
	//sourceReplace(source, "should be adjusted", "hahalol");
	plog(LOG_INFO, "%s\n", source->buffer);
	
	int ret = openclBuildProgram(source->buffer);
	if (ret != 0) {
		//return 1;
	}
	*/
	
	sdlMain();

	//sourceDestroy(source);	
	
	//openclFiniProgram();
	
	openclFini();

	sdlFini();

	return 0;
}

