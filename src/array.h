/*
 * array.h:
 * Generic growing array with O(log2(size)) cost of growing
 *
 * This software may be freely used and distributed according to the terms
 * of the GNU GPL version 2 or 3. See LICENSE for more information.
 *
 * Copyright (c) 2015 David Čepelík <cepelik@gymlit.cz>
 */


#include <stdlib.h>
#include <stdio.h>

#include "common.h"


#ifndef ARRAY_ELEM
#error Please define the ARRAY_ELEM macro prior to including array.h.
#endif

#ifndef ARRAY_PREFIX
#error Please define the ARRAY_PREFIX macro prior to including array.h.
#endif


#define P(x)	ARRAY_PREFIX(x)


#define DEFAULT_CAPACITY	10


struct P(array) {
	ARRAY_ELEM *mry;	/* pointer to managed memory */
	size_t offset;		/* offset to first actual item */
	ARRAY_ELEM *elems;	/* pointer to first actual item */
	size_t size;		/* number of elements in the array */
	size_t capacity;	/* number of elements that fit into the array */
};


static inline void
P(array_push_prepare)(struct P(array) *arr, size_t new_count) {
	size_t new_capacity;

	if (arr->size + arr->offset + new_count > arr->capacity) {
		new_capacity = MAX(2 * arr->capacity, arr->size + arr->offset + new_count);
		DEBUG_MSG("Expanding array capacity from %zu to %zu elements",
			arr->capacity, new_capacity);

		arr->mry = realloc_safe(arr->mry, new_capacity);
		arr->elems = arr->mry + arr->offset;
		arr->capacity = new_capacity;
	}
}


static void
P(array_init_size)(struct P(array) *arr, size_t capacity) {
	arr->mry = NULL;
	arr->offset = 0;
	arr->size = 0;
	arr->capacity = 0;

	P(array_push_prepare)(arr, capacity); /* allocates memory */
}


static void
P(array_init)(struct P(array) *arr) {
	P(array_init_size)(arr, DEFAULT_CAPACITY);
}


static inline void
P(array_push)(struct P(array) *arr, ARRAY_ELEM elem) {
	P(array_push_prepare)(arr, 1);
	arr->elems[arr->size++] = elem;
}


static inline void
P(array_push_n)(struct P(array) *arr, ARRAY_ELEM *new_elems, size_t new_count) {
	size_t i;

	P(array_push_prepare)(arr, new_count);
	for (i = 0; i < new_count; i++) {
	 	arr->elems[arr->size++] = new_elems[i];
	}
}


static inline ARRAY_ELEM
P(array_pop)(struct P(array) *arr) {
	if (arr->size == 0) {
		die("Cannot pop empty array\n");
	}
	
	return arr->elems[--arr->size];
}


static inline ARRAY_ELEM
P(array_shift)(struct P(array) *arr) {
	if (arr->size == 0) {
		die("Cannot shift empty array\n");
	}

	arr->size--;
	arr->offset++;
	return *(arr->elems++);
}


static void
P(array_unshift)(struct P(array) *arr, ARRAY_ELEM elem) {
	die("array_unshift is not supported at the moment\n");
}


static inline ARRAY_ELEM
P(array_first)(struct P(array) *arr) {
	return arr->elems[0];
}


static inline ARRAY_ELEM
P(array_last)(struct P(array) *arr) {
	return arr->elems[arr->size - 1];
}


#undef P

#undef ARRAY_ELEM
#undef ARRAY_PREFIX
