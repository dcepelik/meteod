/*
 * common.h
 * Common functions
 *
 * This software may be freely used and distributed according to the terms
 * of the GNU GPL version 2 or 3. See LICENSE for more information.
 *
 * Copyright (c) 2015 David Čepelík <cepelik@gymlit.cz>
 */


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
