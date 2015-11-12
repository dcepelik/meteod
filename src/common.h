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
#define	COMMON_H

#include <stdlib.h>
#include <stdint.h>


#define	MAX(a, b)		((a) > (b) ? (a) : (b))


#ifndef DEBUG
#define	DEBUG	0
#endif

#define	DEBUG_MSG(fmt, ...) do { \
	if (DEBUG) fprintf(stderr, "*** DEBUG *** " fmt " at %s:%d\n", \
		__VA_ARGS__, __FILE__, __LINE__); \
} while (0);


typedef unsigned char		uchar;
typedef uint32_t		uint_t;
typedef uint64_t		ulong_t;


void *die(char *format, ...);


void *malloc_safe(size_t size);


void *realloc_safe(void *x, size_t size);


#endif
