#include "rrd.h"
#include "wmr.h"
#include "common.h"
#include "strbuf.h"

#include <rrd.h>
#include <time.h>


#define RRD_ROOT_PATH "/var/wmrd/rrd"			// TODO


static void
write_rrd(char *file_rel_path, strbuf *data) {
	strbuf filename;
	strbuf_init(&filename);
	strbuf_append(&filename, "%s/%s", RRD_ROOT_PATH, file_rel_path);

	strbuf_prepend(data, "%li:", time(NULL));

	char *update_params[] = {
		"rrdupdate",
		filename.str,
		data->str,
		NULL
	};

	int ret = rrd_update(3, update_params);
	if (ret != 0) {
		DEBUG_MSG("rrd_update() failed with return code %i\n", ret);
		DEBUG_MSG("rrd_get_error(): %s\n", rrd_get_error());

		rrd_clear_error();
	}

	strbuf_free(&filename);
}


static void
log_wind(wmr_wind *wind) {
	strbuf data;
	strbuf_init(&data);
	strbuf_append(
		&data,
		"%.1f:%.1f",
		wind->avg_speed,
		wind->gust_speed
	);

	write_rrd(WIND_RRD, &data);
	strbuf_free(&data);
}


static void
log_rain(wmr_rain *rain) {
	strbuf data;
	strbuf_init(&data);
	strbuf_append(
		&data,
		"%.1f:%.0f",
		rain->rate,
		rain->accum_2007
	);

	write_rrd(RAIN_RRD, &data);
	strbuf_free(&data);
}


static void
log_uvi(wmr_uvi *uvi) {
	strbuf data;
	strbuf_init(&data);
	strbuf_append(
		&data,
		"%u",
		uvi->index
	);

	write_rrd(UVI_RRD, &data);
	strbuf_free(&data);
}


static void
log_baro(wmr_baro *baro) {
	strbuf data;
	strbuf_init(&data);
	strbuf_append(
		&data,
		"%u:%u",
		baro->pressure,
		baro->alt_pressure
	);

	write_rrd(BARO_RRD, &data);
	strbuf_free(&data);
}


static void
log_temp(wmr_temp *temp) {
	strbuf data;
	strbuf_init(&data);
	strbuf_append(
		&data,
		"%.1f:%u:%.1f",
		temp->temp,
		temp->humidity,
		temp->dew_point
	);

	strbuf filename; // filename depends on sensor ID
	strbuf_init(&filename);
	strbuf_append(&filename, TEMPN_RRD, temp->sensor_id);

	write_rrd(filename.str, &data);

	strbuf_free(&data);
	strbuf_free(&filename);
}


void
log_to_rrd(wmr_reading *reading, char *rrd_file) {
	switch (reading->type) {
	case WIND_DATA:
		log_wind(&reading->wind);
		break;

	case RAIN_DATA:
		log_rain(&reading->rain);
		break;

	case UVI_DATA:
		log_uvi(&reading->uvi);
		break;

	case BARO_DATA:
		log_baro(&reading->baro);
		break;

	case TEMP_DATA:
		log_temp(&reading->temp);
		break;
	}
}



