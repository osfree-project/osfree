#ifndef __STDIO_H_
#define __STDIO_H_

int printf(const char *format, ...);

#define fprintf(a, ...)		printf(__VA_ARGS__)

#endif
