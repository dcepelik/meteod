/*
 * serialize.c:
 * Super-simple WMR200 data serialization/deserialization routines
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

#include <math.h>
#include <endian.h>		/* TODO this ain't POSIX compliant module */


#define	ARRAY_ELEM		unsigned char
#define	ARRAY_PREFIX(x)		byte_##x
#include "array.h"


const double FLOAT_SCALE = (1L << 32);	/* scale float to int */


/*
 * (De)serialization of primitives
 */


static void
serialize_char(struct byte_array *arr, char c) {
	byte_array_push(arr, c);
}


static char
deserialize_char(struct byte_array *arr) {
	return (byte_array_shift(arr));
}


static void
serialize_long(struct byte_array *arr, long l) {
	long nl;
	char sign;

	sign = (l >= 0) ? (0) : (1);
	l = labs(l);

	nl = htobe64(l);
	
	serialize_char(arr, sign);
	byte_array_push_n(arr, (unsigned char *)&nl, sizeof (nl));
}


static long
deserialize_long(struct byte_array *arr) {
	long be64 = 0;
	char sign;
	int i;

	sign = deserialize_char(arr);
	for (i = 0; i < sizeof (be64); i++) {
		((char *)&be64)[i] = byte_array_shift(arr);
	}

	return (((sign == 0) ? (+1) : (-1)) * be64toh(be64));
}


static void
serialize_int(struct byte_array *arr, int i) {
	serialize_long(arr, (long)i);
}


static int
deserialize_int(struct byte_array *arr) {
	return ((int)deserialize_long(arr));
}


void
serialize_float(struct byte_array *arr, float f) {
	float fract;
	int exp;
	long ifract;
	char sign;

	sign = (f >= 0) ? (0) : (1);
	f = fabs(f);

	/* get fractional part and exponent */
	fract = frexpf(f, &exp);

	/* convert fractional part to integer */
	ifract = trunc(fract * FLOAT_SCALE);

	serialize_char(arr, sign);
	serialize_long(arr, ifract);
	serialize_int(arr, exp);
}


float
deserialize_float(struct byte_array *arr) {
	float fract;
	int exp;
	long ifract;
	char sign;

	sign = deserialize_char(arr);
	ifract = deserialize_long(arr);
	exp = deserialize_int(arr);

	fract = ifract / FLOAT_SCALE;
	return ((sign == 0 ? (+1) : (-1)) * ldexpf(fract, exp));

	return 0;
}


static void
serialize_string(struct byte_array *arr, const char *str) {
	if (str != NULL) {
		int i = 0;
		while (str[i] != '\0') {
			serialize_char(arr, str[i]);
			i++;
		}
	}

	serialize_char(arr, '\0');
}


static char *
deserialize_string(struct byte_array *arr) {
	strbuf str;

	strbuf_init(&str);
	while (byte_array_first(arr) != '\0') {
		strbuf_append(&str, "%c", deserialize_char(arr));
	}

	(void) deserialize_char(arr); /* dump the '\0' */

	char *out_str = strbuf_copy(&str);
	strbuf_free(&str);

	return (out_str);
}


/*
 * (De)serialization of WMR data
 */


static void
serialize_wind(struct byte_array *arr, wmr_wind *wind) {
	serialize_string(arr, wind->dir);
	serialize_float(arr, wind->gust_speed);
	serialize_float(arr, wind->avg_speed);
	serialize_float(arr, wind->chill);
}


static void
deserialize_wind(struct byte_array *arr, wmr_wind *wind) {
	wind->dir = deserialize_string(arr);
	wind->gust_speed = deserialize_float(arr);
	wind->avg_speed = deserialize_float(arr);
	wind->chill = deserialize_float(arr);
}


static void
serialize_rain(struct byte_array *arr, wmr_rain *rain) {
	serialize_float(arr, rain->rate);
	serialize_float(arr, rain->accum_hour);
	serialize_float(arr, rain->accum_24h);
	serialize_float(arr, rain->accum_2007);
}


static void
deserialize_rain(struct byte_array *arr, wmr_rain *rain) {
	rain->rate = deserialize_float(arr);
	rain->accum_hour = deserialize_float(arr);
	rain->accum_24h = deserialize_float(arr);
	rain->accum_2007 = deserialize_float(arr);
}


static void
serialize_uvi(struct byte_array *arr, wmr_uvi *uvi) {
	serialize_int(arr, uvi->index);
}


static void
deserialize_uvi(struct byte_array *arr, wmr_uvi *uvi) {
	uvi->index = deserialize_int(arr);
}


static void
serialize_baro(struct byte_array *arr, wmr_baro *baro) {
	serialize_int(arr, baro->pressure);
	serialize_int(arr, baro->alt_pressure);
	serialize_string(arr, baro->forecast);
}


static void
deserialize_baro(struct byte_array *arr, wmr_baro *baro) {
	baro->pressure = deserialize_int(arr);
	baro->alt_pressure = deserialize_int(arr);
	baro->forecast = deserialize_string(arr);
}


static void
serialize_temp(struct byte_array *arr, wmr_temp *temp) {
	serialize_int(arr, temp->sensor_id);
	serialize_int(arr, temp->humidity);
	serialize_int(arr, temp->heat_index);
	serialize_float(arr, temp->temp);
	serialize_float(arr, temp->dew_point);
}


static void
deserialize_temp(struct byte_array *arr, wmr_temp *temp) {
	temp->sensor_id = deserialize_int(arr);
	temp->humidity = deserialize_int(arr);
	temp->heat_index = deserialize_int(arr);
	temp->temp = deserialize_float(arr);
	temp->dew_point = deserialize_float(arr);
}


static void
serialize_status(struct byte_array *arr, wmr_status *status) {
	serialize_string(arr, status->wind_bat);
	serialize_string(arr, status->temp_bat);
	serialize_string(arr, status->rain_bat);
	serialize_string(arr, status->uv_bat);

	serialize_string(arr, status->wind_sensor);
	serialize_string(arr, status->temp_sensor);
	serialize_string(arr, status->rain_sensor);
	serialize_string(arr, status->uv_sensor);

	serialize_string(arr, status->rtc_signal_level);
}


static void
deserialize_status(struct byte_array *arr, wmr_status *status) {
	status->wind_bat = deserialize_string(arr);
	status->temp_bat = deserialize_string(arr);
	status->rain_bat = deserialize_string(arr);
	status->uv_bat = deserialize_string(arr);

	status->wind_sensor = deserialize_string(arr);
	status->temp_sensor = deserialize_string(arr);
	status->rain_sensor = deserialize_string(arr);
	status->uv_sensor = deserialize_string(arr);

	status->rtc_signal_level = deserialize_string(arr);
}


static void
serialize_meta(struct byte_array *arr, wmr_meta *meta) {
	serialize_int(arr, meta->num_packets);
	serialize_int(arr, meta->num_failed);
	serialize_int(arr, meta->num_frames);
	serialize_float(arr, meta->error_rate);
	serialize_long(arr, meta->num_bytes);
	serialize_long(arr, meta->latest_packet);
}


static void
deserialize_meta(struct byte_array *arr, wmr_meta *meta) {
	meta->num_packets = deserialize_int(arr);
	meta->num_failed = deserialize_int(arr);
	meta->num_frames = deserialize_int(arr);
	meta->error_rate = deserialize_float(arr);
	meta->num_bytes = deserialize_long(arr);
	meta->latest_packet = deserialize_long(arr);
}


static void
serialize_reading(struct byte_array *arr, wmr_reading *reading) {
	serialize_int(arr, reading->type);
	serialize_long(arr, reading->time);

	switch (reading->type) {
	case WMR_WIND:
		serialize_wind(arr, &reading->wind);
		break;

	case WMR_RAIN:
		serialize_rain(arr, &reading->rain);
		break;

	case WMR_UVI:
		serialize_uvi(arr, &reading->uvi);
		break;

	case WMR_BARO:
		serialize_baro(arr, &reading->baro);
		break;

	case WMR_TEMP:
		serialize_temp(arr, &reading->temp);
		break;

	case WMR_STATUS:
		serialize_status(arr, &reading->status);
		break;

	case WMR_META:
		serialize_meta(arr, &reading->meta);
		break;

	default:
		die("Cannot serialize reading of type %02x\n", reading->type);
	}
}


static void
deserialize_reading(struct byte_array *arr, wmr_reading *reading) {
	reading->type = deserialize_int(arr);
	reading->time = deserialize_long(arr);

	switch (reading->type) {
	case WMR_WIND:
		deserialize_wind(arr, &reading->wind);
		break;

	case WMR_RAIN:
		deserialize_rain(arr, &reading->rain);
		break;

	case WMR_UVI:
		deserialize_uvi(arr, &reading->uvi);
		break;

	case WMR_BARO:
		deserialize_baro(arr, &reading->baro);
		break;

	case WMR_TEMP:
		deserialize_temp(arr, &reading->temp);
		break;

	case WMR_STATUS:
		deserialize_status(arr, &reading->status);
		break;

	case WMR_META:
		deserialize_meta(arr, &reading->meta);
		break;

	default:
		die("Cannot serialize reading of type %02x\n", reading->type);
	}
}


/*
 * public interface
 */


void
serialize_data(struct byte_array *arr, wmr_latest_data *data) {
	serialize_reading(arr, &data->wind);
	serialize_reading(arr, &data->rain);
	serialize_reading(arr, &data->uvi);
	serialize_reading(arr, &data->baro);
	//serialize_reading(arr, &data->temp);
	serialize_reading(arr, &data->status);
	//serialize_reading(arr, &data->meta);
}


void
deserialize_data(struct byte_array *arr, wmr_latest_data *data) {
	deserialize_reading(arr, &data->wind);
	deserialize_reading(arr, &data->rain);
	deserialize_reading(arr, &data->uvi);
	deserialize_reading(arr, &data->baro);
	//deserialize_reading(arr, &data->temp);
	deserialize_reading(arr, &data->status);
	//deserialize_reading(arr, &data->meta);

}
