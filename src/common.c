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
	exit(1);
}


void *
malloc_safe(size_t size) {
	void *x = malloc(size);
	if (!x) die("Cannot allocate %zu bytes of memory", size);

	return x;
}


void *
realloc_safe(void *x, size_t size) {
	x = realloc(x, size);
	if (!x) die("Cannot reallocate memory (new size was %zu bytes)", size);

	return x;
}



