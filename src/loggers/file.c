/*
 * loggers/file.c:
 * Log WMR readings to a file
 *
 * This software may be freely used and distributed according to the terms
 * of the GNU GPL version 2 or 3. See LICENSE for more information.
 *
 * Copyright (c) 2015 David Čepelík <cepelik@gymlit.cz>
 */


#include "file.h"
#include "wmrdata.h"


static void
log_wind(wmr_wind *wind, FILE *stream) {
	fprintf(stream, "wind %li {\n", wind->time);
	fprintf(stream, "\twind.dir: %s\n", wind->dir);
	fprintf(stream, "\twind.gust_speed: %.2f\n", wind->gust_speed);
	fprintf(stream, "\twind.avg_speed: %.2f\n", wind->avg_speed);
	fprintf(stream, "\twind.chill: %.1f\n", wind->chill);
	fprintf(stream, "}\n\n");
}


static void
log_rain(wmr_rain *rain, FILE *stream) {
	fprintf(stream, "rain %li {\n", rain->time);
	fprintf(stream, "\train.rate: %.2f\n", rain->rate);
	fprintf(stream, "\train.accum_hour: %.2f\n", rain->accum_hour);
	fprintf(stream, "\train.accum_24h: %.2f\n", rain->accum_24h);
	fprintf(stream, "\train.accum_2007: %.2f\n", rain->accum_2007);
	fprintf(stream, "}\n\n");
}


static void
log_uvi(wmr_uvi *uvi, FILE *stream) {
	fprintf(stream, "uvi %li {\n", uvi->time);
	fprintf(stream, "\tuvi.index: %u\n", uvi->index);
	fprintf(stream, "}\n\n");
}


static void
log_baro(wmr_baro *baro, FILE *stream) {
	fprintf(stream, "baro %li {\n", baro->time);
	fprintf(stream, "\tbaro.pressure: %u\n", baro->pressure);
	fprintf(stream, "\tbaro.alt_pressure: %u\n", baro->alt_pressure);
	fprintf(stream, "\tbaro.forecast: %s\n", baro->forecast);
	fprintf(stream, "}\n\n");
}


static void
log_temp(wmr_temp *temp, FILE *stream) {
	uint id = temp->sensor_id;

	fprintf(stream, "temp%u %li {\n", temp->sensor_id, temp->time);
	fprintf(stream, "\ttemp.%u.humidity: %u\n", id, temp->humidity);
	fprintf(stream, "\ttemp.%u.heat_index: %u\n", id, temp->heat_index);
	fprintf(stream, "\ttemp.%u.temp: %.1f\n", id, temp->temp);
	fprintf(stream, "\ttemp.%u.dew_point: %.1f\n", id, temp->dew_point);
	fprintf(stream, "}\n\n");
}


static void
log_status(wmr_status *status, FILE *stream) {
	fprintf(stream, "status %li {\n", status->time);

	fprintf(stream, "\tstatus.wind_bat: %s\n", status->wind_bat);
	fprintf(stream, "\tstatus.temp_bat: %s\n", status->temp_bat);
	fprintf(stream, "\tstatus.rain_bat: %s\n", status->rain_bat);
	fprintf(stream, "\tstatus.uv_bat: %s\n", status->uv_bat);

	fprintf(stream, "\tstatus.wind_sensor: %s\n", status->wind_sensor);
	fprintf(stream, "\tstatus.temp_sensor: %s\n", status->temp_sensor);
	fprintf(stream, "\tstatus.rain_sensor: %s\n", status->rain_sensor);
	fprintf(stream, "\tstatus.uv_sensor: %s\n", status->uv_sensor);

	fprintf(stream, "\tstatus.rtc_signal_level: %s\n", status->rtc_signal_level);

	fprintf(stream, "}\n\n");
}


void
log_to_file(wmr_reading *reading, FILE *stream) {
	switch (reading->type) {
	case WMR_WIND:
		log_wind(&reading->wind, stream);
		break;

	case WMR_RAIN:
		log_rain(&reading->rain, stream);
		break;

	case WMR_UVI:
		log_uvi(&reading->uvi, stream);
		break;

	case WMR_BARO:
		log_baro(&reading->baro, stream);
		break;

	case WMR_TEMP:
		log_temp(&reading->temp, stream);
		break;

	case WMR_STATUS:
		log_status(&reading->status, stream);
		break;
	}
}



