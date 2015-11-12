/*
 * common.c:
 * Common functions
 *
 * This software may be freely used and distributed according to the terms
 * of the GNU GPL version 2 or 3. See LICENSE for more information.
 *
 * Copyright (c) 2015 David Čepelík <cepelik@gymlit.cz>
 */


#include "common.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>


void *
die(char *format, ...) {
	va_list args;
	va_start(args, format);

	vprintf(format, args);

	va_end(args);
	exit(EXIT_FAILURE);
}


void *
malloc_safe(size_t size) {
	void *x = malloc(size);
	if (!x) die("Cannot allocate %zu bytes of memory", size);

	return (x);
}


void *
realloc_safe(void *x, size_t size) {
	x = realloc(x, size);
	if (!x) die("Cannot reallocate memory (new size was %zu bytes)", size);

	return (x);
}
