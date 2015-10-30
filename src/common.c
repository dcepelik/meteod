#include "common.h"


void *
malloc_safe(size_t size) {
	void *x = malloc(size);
	if (x == NULL) {
		fprintf(stderr, "Cannot allocate %zu bytes of memory\n", size);
		exit(1);
	}

	return x;
}



