#ifndef SOURCE_H
#define SOURCE_H

typedef struct {
	char *buffer;
	size_t bufferSize;
	size_t len;
} Source;


Source *sourceCreate();
Source *sourceLoad();
void sourceAppend(Source *src, char *append);
Source *sourceLoad(const char *fn);


#endif

