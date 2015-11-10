/*
 * serialize.c:
 * Super-simple data serialization/deserialization routines
 *
 * This software may be freely used and distributed according to the terms
 * of the GNU GPL version 2 or 3. See LICENSE for more information.
 *
 * Copyright (c) 2015 David Čepelík <cepelik@gymlit.cz>
 */


#include "serialize.h"
#include "common.h"

#include <arpa/inet.h>


#define ARRAY_ELEM		unsigned char
#define ARRAY_PREFIX(x)		byte_##x
#include "array.h"


void
serialize_int(struct byte_array *arr, int i) {
	serialize_long(arr, (long)i);
}


void
serialize_long(struct byte_array *arr, long l) {
	int nl = htonl(l);
	byte_array_push_n(arr, (unsigned char *)&nl, sizeof(l));
}


void
serialize_char(struct byte_array *arr, char c) {
	byte_array_push(arr, c);
}


void
serialize_float(struct byte_array *arr, float f) {
}


int
deserialize_int(struct byte_array *arr) {
	return (int)deserialize_long(arr);
}


long
deserialize_long(struct byte_array *arr) {
	int i;

	long nl = 0;
	for (i = 0; i < sizeof(nl); i++) {
		nl *= 256;
		nl += byte_array_pop(arr);
	}

	return ntohl(nl);
}


char
deserialize_char(struct byte_array *arr) {
	return byte_array_pop(arr);
}


float
deserialize_float(struct byte_array *arr) {
	die("not impl");
	return 0;
}
