#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <assert.h>
#include <sys/stat.h>
#include "log.h"
#include "source.h"


Source *sourceCreate() {
	Source *src = malloc(sizeof(Source));
	src->buffer = malloc(512 * sizeof(char));
	src->buffer[0] = '\0';
	src->bufferSize = 512;
	src->len = 0;
	return src;
}

void sourceDestroy(Source *src) {
	free(src->buffer);
	free(src);
}

void sourceAppend(Source *src, char *append) {
	size_t appendLen = strlen(append);
	size_t lenRequired = src->len + appendLen + 1;
	if (lenRequired > src->bufferSize) {
		src->buffer = realloc(src->buffer, lenRequired);
		if (src->buffer == NULL) {
			plog(LOG_ERROR, "sourceAppend(): memory allocated failed\n");
			return;
		}
	}
	strcat(src->buffer, append);
}

/*
void sourceReplace(Source *src, char *needle, char *replacement) {
	assert(strstr(needle, replacement) == NULL);

}
*/

// load openCL source file of filename fn into memory
// caller is responsible for free()ing this memory
Source *sourceLoad(const char *fn) {
	struct stat fstats;
	FILE *file;
	size_t size;

	// get file size
	if (stat(fn, &fstats) == 0) {
		size = fstats.st_size;
	} else {
		plog(LOG_ERROR, "failed to stat file: %s: %s\n", fn, strerror(errno));
		return NULL;
	}

	Source *src = malloc(sizeof(Source));
	if (src == NULL) {
		plog(LOG_ERROR, "memory allocation failed: %s\n", strerror(errno));
		return NULL;
	}

	// allocate the memory
	src->buffer = malloc(size + 1);
	if (src->buffer == NULL) {
		plog(LOG_ERROR, "memory allocation failed: %s\n", strerror(errno));
		free(src);
		return NULL;
	}
	
	// open the file
	file = fopen(fn, "rb");
	if (file == NULL) {
		plog(LOG_ERROR, "failed to open file: %s: %s\n", fn, strerror(errno));
		free(src->buffer);
		free(src);
		return NULL;
	}
	
	// read it into memory
	size_t c = fread(src->buffer, 1, size, file);
	if (c != size) {
		plog(LOG_ERROR, "file read error. %zu of %zu bytes read: %s\n", c, size, strerror(errno));
		perror("fread");
		free(src->buffer);
		free(src);
		return NULL;
	}
	
	// null terminate the source
	src->buffer[size] = '\0';
	src->bufferSize = size;
	src->len = size;
	
	fclose(file);
	
	return src;
}

