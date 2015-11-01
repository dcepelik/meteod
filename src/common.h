#ifndef COMMON_H
#define COMMON_H

#include <stdlib.h>


typedef unsigned int		uint;
typedef unsigned char		uchar;


#ifdef  DEBUG
#define DEBUG_MSG(...)		fprintf(stderr, __VA_ARGS__)
#else
#define DEBUG_MSG(...)
#endif


void *die(char *format, ...);


void *malloc_safe(size_t size);


void *realloc_safe(void *x, size_t size);


#endif
