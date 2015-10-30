#ifndef COMMON_H
#define COMMON_H

#include <stdio.h>
#include <stdlib.h>


typedef unsigned int		uint;
typedef unsigned char		uchar;


#ifdef  DEBUG
#define DEBUG_MSG(...)		fprintf(stderr, __VA_ARGS__)
#else
#define DEBUG_MSG(...)
#endif


#endif
