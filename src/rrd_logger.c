#include "rrd_logger.h"
#include "wmr200.h"
#include "time.h"
#include "strbuf.h"
#include "common.h"

#include <stdarg.h>
#include <rrd.h>


#define RRD_ROOT_PATH "/var/wmrd/rrd"


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
log_wind(struct wmr_wind_reading *wind) {
	strbuf data;
	strbuf_init(&data);
	strbuf_append(
		&data,
		"%.1f:%.1f",
		wind->avg_speed,
		wind->gust_speed
	);

	write_rrd("wind.rrd", &data);
	strbuf_free(&data);
}


static void
log_temp(struct wmr_temp_reading *temp) {
	strbuf data;
	strbuf_init(&data);
	strbuf_append(&data, "%.1f", temp->temp);

	strbuf filename; // filename depends on sensor ID
	strbuf_init(&filename);
	strbuf_append(&filename, "temp%u.rrd", temp->sensor_id);

	write_rrd(filename.str, &data);

	strbuf_free(&data);
	strbuf_free(&filename);
}


void
log_to_rrd(struct wmr_reading *reading, char *rrd_file) {
	switch (reading->type) {
	case WIND_DATA:
		log_wind(&reading->wind);
		break;

	case TEMP_DATA:
		log_temp(&reading->temp);
		break;
	}
}


