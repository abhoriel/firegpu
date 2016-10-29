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
	//src->len = 0;
	return src;
}

void sourceDestroy(Source *src) {
	free(src->buffer);
	free(src);
}

static int expandBufferIfNecessary(Source *src, size_t lenRequired) {
	if (lenRequired > src->bufferSize) {
		src->buffer = realloc(src->buffer, lenRequired);
		if (src->buffer == NULL) {
			plog(LOG_ERROR, "sourceAppend(): memory allocated failed\n");
			return 1;
		}
	}
	return 0;
}

void sourceAppend(Source *src, const char *append) {
	size_t appendLen = strlen(append);
	size_t lenRequired = strlen(src->buffer) + appendLen + 1;
	expandBufferIfNecessary(src, lenRequired);
	strcat(src->buffer, append);
}

void sourceReplace(Source *src, const char *needle, const char *replacement) {
	assert(strstr(needle, replacement) == NULL);
	char *found;
	size_t needleLen = strlen(needle);
	size_t replacementLen = strlen(replacement);
	while((found = strstr(src->buffer, needle)) != NULL) {
		size_t srcLen = strlen(src->buffer);
		expandBufferIfNecessary(src, srcLen + 1 + (replacementLen - needleLen));
		size_t moveLen = 1 + srcLen - ((found - src->buffer) + needleLen);
		memmove(found + replacementLen, found + needleLen, moveLen);
		memcpy(found, replacement, replacementLen);

	}
}

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
	
	fclose(file);
	
	return src;
}

