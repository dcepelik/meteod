/*
 * Common helpers
 */

#include "common.h"
#include "log.h"

#include <stdlib.h>

void *realloc_safe(void *x, size_t size)
{
	x = realloc(x, size);
	if (!x)
		log_exit("Cannot allocate %zu bytes of memory", size);
	return x;
}

void *malloc_safe(size_t size)
{
	return realloc_safe(NULL, size);
}
