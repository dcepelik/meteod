#include "file_logger.h"
#include "wmr200.h"

#include <stdio.h>


static void
handle_wind(struct wmr_wind_reading *wind, FILE *stream) {
	fprintf(stream, "\twind.dir: %s\n", wind->dir);
	fprintf(stream, "\twind.gust_speed: %.2f\n", wind->gust_speed);
	fprintf(stream, "\twind.avg_speed: %.2f\n", wind->avg_speed);
	fprintf(stream, "\twind.chill: %.1f\n", wind->chill);
}


static void
handle_rain(struct wmr_rain_reading *rain, FILE *stream) {
	fprintf(stream, "\train.rate: %.2f\n", rain->rate);
	fprintf(stream, "\train.accum_hour: %.2f\n", rain->accum_hour);
	fprintf(stream, "\train.accum_24h: %.2f\n", rain->accum_24h);
	fprintf(stream, "\train.accum_2007: %.2f\n", rain->accum_2007);
}


static void
handle_uvi(struct wmr_uvi_reading *uvi, FILE *stream) {
	fprintf(stream, "\tuvi.index: %u\n", uvi->index);
}


static void
handle_baro(struct wmr_baro_reading *baro, FILE *stream) {
	fprintf(stream, "\tbaro.pressure: %u\n", baro->pressure);
	fprintf(stream, "\tbaro.alt_pressure: %u\n", baro->alt_pressure);
	fprintf(stream, "\tbaro.forecast: %s\n", baro->forecast);
}


static void
handle_temp(struct wmr_temp_reading *temp, FILE *stream) {
	uint id = temp->sensor_id;

	fprintf(stream, "\ttemp.%u.humidity: %u\n", id, temp->humidity);
	fprintf(stream, "\ttemp.%u.heat_index: %u\n", id, temp->heat_index);
	fprintf(stream, "\ttemp.%u.temp: %.1f\n", id, temp->temp);
	fprintf(stream, "\ttemp.%u.dew_point: %.1f\n", id, temp->dew_point);
}


static void
handle_status(struct wmr_status_reading *status, FILE *stream) {
	fprintf(stream, "\tstatus.wind_bat: %s\n", status->wind_bat);
	fprintf(stream, "\tstatus.temp_bat: %s\n", status->temp_bat);
	fprintf(stream, "\tstatus.rain_bat: %s\n", status->rain_bat);
	fprintf(stream, "\tstatus.uv_bat: %s\n", status->uv_bat);

	fprintf(stream, "\tstatus.wind_sensor: %s\n", status->wind_sensor);
	fprintf(stream, "\tstatus.temp_sensor: %s\n", status->temp_sensor);
	fprintf(stream, "\tstatus.rain_sensor: %s\n", status->rain_sensor);
	fprintf(stream, "\tstatus.uv_sensor: %s\n", status->uv_sensor);

	fprintf(stream, "\tstatus.rtc_signal_level: %s\n", status->rtc_signal_level);
}


void
log_to_file(struct wmr_reading *reading, FILE *stream) {
	fprintf(stream, "reading %li {\n", reading->time);

	switch (reading->type) {
	case WIND_DATA:
		handle_wind(&reading->wind, stream);
		break;

	case RAIN_DATA:
		handle_rain(&reading->rain, stream);
		break;

	case UVI_DATA:
		handle_uvi(&reading->uvi, stream);
		break;

	case BARO_DATA:
		handle_baro(&reading->baro, stream);
		break;

	case TEMP_DATA:
		handle_temp(&reading->temp, stream);
		break;

	case STATUS_DATA:
		handle_status(&reading->status, stream);
		break;
	}

	fprintf(stream, "}\n\n");
}



