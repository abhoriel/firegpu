#ifndef SOURCE_H
#define SOURCE_H

#include <stddef.h>

typedef struct {
	char *buffer;
	size_t bufferSize;
	//size_t len;
} Source;


Source *sourceCreate();
void sourceDestroy(Source *src);
Source *sourceLoad();
void sourceAppend(Source *src, const char *append);
void sourceReplace(Source *src, const char *needle, const char *replacement);
void sourceAppendFormatted(Source *src, const char *format, ...);
void sourceReplaceFormatted(Source *src, const char *needle, const char *format, ...);
Source *sourceLoad(const char *fn);


#endif

