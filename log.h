#ifndef LOG_H
#define LOG_H

#define LOG_VERBOSE     0
#define LOG_INFO        1
#define LOG_WARN        2
#define LOG_ERROR       3

void plog(int level, const char *format, ...);

#endif	// LOG_H
