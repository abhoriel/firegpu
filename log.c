#include <stdio.h>
#include <stdarg.h>
#include "log.h"

#define LOG_LEVEL   0

extern int verbose;

void plog(int level, const char *format, ...) {
	va_list args;
	if (level < LOG_LEVEL) {
		return;
	}
	if ((level == LOG_VERBOSE) && (!verbose)) {
		return;
	}
	va_start(args, format);
	if (level == LOG_ERROR) {
		vfprintf(stderr, format, args);
	} else {
		vfprintf(stdout, format, args);
	}
	va_end(args);
}

