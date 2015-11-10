/*
 * serialize.h:
 * Super-simple data serialization/deserialization routines
 *
 * This software may be freely used and distributed according to the terms
 * of the GNU GPL version 2 or 3. See LICENSE for more information.
 *
 * Copyright (c) 2015 David Čepelík <cepelik@gymlit.cz>
 */


#ifndef SERIALIZE_H
#define SERIALIZE_H


struct byte_array;	/* growing byte array */


void serialize_int(struct byte_array *arr, int i);

void serialize_long(struct byte_array *arr, long l);

void serialize_char(struct byte_array *arr, char c);

void serialize_float(struct byte_array *arr, float f);

int deserialize_int(struct byte_array *arr);

long deserialize_long(struct byte_array *arr);

char deserialize_char(struct byte_array *arr);

float deserialize_float(struct byte_array *arr);


#endif
