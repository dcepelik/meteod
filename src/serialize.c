/*
 * serialize.c:
 * Super-simple WMR200 arr serialization/deserialization routines
 *
 * This software may be freely used and distributed according to the terms
 * of the GNU GPL version 2 or 3. See LICENSE for more information.
 *
 * Copyright (c) 2015 David Čepelík <cepelik@gymlit.cz>
 */


#include "serialize.h"
#include "wmrdata.h"
#include "strbuf.h"
#include "common.h"

#include <endian.h>		/* TODO this ain't compatible */


#define ARRAY_ELEM		unsigned char
#define ARRAY_PREFIX(x)		byte_##x
#include "array.h"


/********************** (de)serialization of primitives  **********************/


static void
serialize_long(struct byte_array *arr, long l) {
	long nl = htobe64(l);
	byte_array_push_n(arr, (unsigned char *)&nl, sizeof(nl));
}


static long
deserialize_long(struct byte_array *arr) {
	long be64 = 0;
	int i;

	for (i = 0; i < sizeof(be64); i++) {
		((char *)&be64)[i] = byte_array_shift(arr);
	}

	return be64toh(be64);
}


static void
serialize_int(struct byte_array *arr, int i) {
	serialize_long(arr, (long)i);
}


static int
deserialize_int(struct byte_array *arr) {
	return (int)deserialize_long(arr);
}


static void
serialize_char(struct byte_array *arr, char c) {
	byte_array_push(arr, c);
}


static char
deserialize_char(struct byte_array *arr) {
	return byte_array_pop(arr);
}


static void
serialize_float(struct byte_array *arr, float f) {
}


static float
deserialize_float(struct byte_array *arr) {
	return 0;
}


static void
serialize_string(struct byte_array *arr, const char *str) {
	int i = 0;
	while (str[i] != '\0') {
		byte_array_push(arr, str[i]);
		i++;
	}
	byte_array_push(arr, '\0');
}


static char *
deserialize_string(struct byte_array *arr) {
	strbuf str;

	strbuf_init(&str);
	while (byte_array_first(arr) != '\0') {
		strbuf_append(&str, "%c", byte_array_shift(arr));
	}

	byte_array_shift(arr); /* dump the '\0' */

	char *out_str = strbuf_copy(&str);
	strbuf_free(&str);

	return out_str;
}


/********************** (de)serialization of WMR data **********************/


void
serialize_wind(struct byte_array *arr, wmr_wind *wind) {
	serialize_long(arr, (long)wind->time);
	serialize_string(arr, wind->dir);
	serialize_float(arr, wind->gust_speed);
	serialize_float(arr, wind->avg_speed);
	serialize_float(arr, wind->chill);
}


wmr_wind
deserialize_wind(struct byte_array *arr) {
	wmr_wind wind = {
		.time = deserialize_long(arr),
		.dir = deserialize_string(arr),
		.gust_speed = deserialize_float(arr),
		.avg_speed = deserialize_float(arr),
		.chill = deserialize_float(arr)
	};

	return wind;
}
