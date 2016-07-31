/*
 * loggers/rrd.c:
 * Log readings to RRD files
 *
 * This software may be freely used and distributed according to the terms
 * of the GNU GPL version 2 or 3. See LICENSE for more information.
 *
 * Copyright (c) 2015 David Čepelík <cepelik@gymlit.cz>
 */


#include "common.h"
#include "log.h"
#include "rrd.h"
#include "strbuf.h"
#include "wmrdata.h"

#include <rrd.h>
#include <time.h>


static void
write_rrd(char *file_rel_path, char *rrd_root, strbuf *data)
{
	strbuf filename;
	strbuf_init(&filename);
	strbuf_append(&filename, "%s/%s", rrd_root, file_rel_path);

	strbuf_prepend(data, "%li:", time(NULL));

	char *update_params[] = {
		"rrdupdate",
		filename.str,
		data->str,
		NULL
	};

	int ret = rrd_update(3, update_params);
	if (ret != 0) {
		log_error("rrd_update: failed with return code %i", ret);
		log_error("rrd_get_error: %s", rrd_get_error());

		rrd_clear_error();
	}

	strbuf_free(&filename);
}


static void
log_wind(wmr_wind *wind, char *rrd_root)
{
	strbuf data;
	strbuf_init(&data);
	strbuf_append(
		&data,
		"%.1f:%.1f",
		wind->avg_speed,
		wind->gust_speed);

	write_rrd(WIND_RRD, rrd_root, &data);
	strbuf_free(&data);
}


static void
log_rain(wmr_rain *rain, char *rrd_root)
{
	strbuf data;
	strbuf_init(&data);
	strbuf_append(
		&data,
		"%.1f:%.0f",
		rain->rate,
		rain->accum_2007);

	write_rrd(RAIN_RRD, rrd_root, &data);
	strbuf_free(&data);
}


static void
log_uvi(wmr_uvi *uvi, char *rrd_root)
{
	strbuf data;
	strbuf_init(&data);
	strbuf_append(
		&data,
		"%u",
		uvi->index);

	write_rrd(UVI_RRD, rrd_root, &data);
	strbuf_free(&data);
}


static void
log_baro(wmr_baro *baro, char *rrd_root)
{
	strbuf data;
	strbuf_init(&data);
	strbuf_append(
		&data,
		"%u:%u",
		baro->pressure,
		baro->alt_pressure);

	write_rrd(BARO_RRD, rrd_root, &data);
	strbuf_free(&data);
}


static void
log_temp(wmr_temp *temp, char *rrd_root)
{
	strbuf data;
	strbuf_init(&data);
	strbuf_append(
		&data,
		"%.1f:%u:%.1f",
		temp->temp,
		temp->humidity,
		temp->dew_point);

	strbuf filename; /* filename depends on sensor ID */
	strbuf_init(&filename);
	strbuf_append(&filename, TEMPN_RRD, temp->sensor_id);

	write_rrd(filename.str, rrd_root, &data);

	strbuf_free(&data);
	strbuf_free(&filename);
}


static void
log_reading(wmr_reading *reading, char *rrd_root)
{
	switch (reading->type) {
	case WMR_WIND:
		log_wind(&reading->wind, rrd_root);
		break;

	case WMR_RAIN:
		log_rain(&reading->rain, rrd_root);
		break;

	case WMR_UVI:
		log_uvi(&reading->uvi, rrd_root);
		break;

	case WMR_BARO:
		log_baro(&reading->baro, rrd_root);
		break;

	case WMR_TEMP:
		log_temp(&reading->temp, rrd_root);
		break;
	}
}


/*
 * public interface
 */


void
rrd_push_reading(wmr_reading *reading, void *arg)
{
	char *rrd_root = (char *)arg;
	log_reading(reading, rrd_root);
}
