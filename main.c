#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <errno.h>
#include <time.h>
#include <sys/stat.h>
#include "opencl.h"
#include "sdl.h"
#include "rng.h"
#include "log.h"


//static char *loadSourceFile(const char *fn);

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
	char fn[] = "brotkernel.cl";
	char *source = NULL;

	int desiredPlatform = 0;
	int desiredDevice = 0;
	(void)argc; (void)argv;	// avoid unused variable compiler warnings
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

	/*
	for (int i = 0; i < 25; i++) {
		printf("%.9g\n", rngGenerateFloat(-1.f, 1.f));
	}
	*/

	long seed = time(NULL);
	plog(LOG_INFO, "rng seed: %ld\n", seed);
	rngSeed(seed);	

	if (sdlInit(w, h) != 0) {
		plog(LOG_ERROR, "error initialising SDL\n");
		return 1;
	}

	(void)source; (void)desiredDevice; (void)desiredPlatform; (void)fn;
	/*
	if (openclInit(desiredPlatform, desiredDevice) != 0) {
		plog(LOG_ERROR, "error initialising openCL\n");
		return 1;
	}
	
	source = loadSourceFile(fn);
	if (source == NULL) {
		return 1;
	}
	
	int ret = openclBuildProgram(source);
	if (ret != 0) {
		return 1;
	}
	*/
	
	sdlMain();

	/*	
	free(source);
	
	openclFiniProgram();
	
	openclFini();
	*/
	sdlFini();

	return 0;
}

#if 0
// load openCL source file of filename fn into memory
// caller is responsible for free()ing this memory
static char *loadSourceFile(const char *fn) {
	struct stat fstats;
	FILE *file;
	char *source;
	size_t size;

	// get file size
	if (stat(fn, &fstats) == 0) {
		size = fstats.st_size;
	} else {
		plog(LOG_ERROR, "failed to stat file: %s: %s\n", fn, strerror(errno));
		return NULL;
	}
	
	// allocate the memory
	source = malloc(size + 1);
	if (source == NULL) {
		plog(LOG_ERROR, "memory allocation failed: %s\n", strerror(errno));
		return NULL;
	}
	
	// open the file
	file = fopen(fn, "rb");
	if (file == NULL) {
		plog(LOG_ERROR, "failed to open file: %s: %s\n", fn, strerror(errno));
		return NULL;
	}
	
	// read it into memory
	size_t c = fread(source, 1, size, file);
	if (c != size) {
		//cerr << "file read error. " << c << " of " << size << " bytes read" << endl;
		plog(LOG_ERROR, "file read error. %zu of %zu bytes read: %s\n", c, size, strerror(errno));
		perror("fread");
		return NULL;
	}
	
	// null terminate the source
	source[size] = '\0';
	
	fclose(file);
	
	return source;
}


#endif
