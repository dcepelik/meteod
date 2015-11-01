#include "file.h"
#include "wmrdata.h"


static void
log_wind(wmr_wind *wind, FILE *stream) {
	fprintf(stream, "\twind.dir: %s\n", wind->dir);
	fprintf(stream, "\twind.gust_speed: %.2f\n", wind->gust_speed);
	fprintf(stream, "\twind.avg_speed: %.2f\n", wind->avg_speed);
	fprintf(stream, "\twind.chill: %.1f\n", wind->chill);
}


static void
log_rain(wmr_rain *rain, FILE *stream) {
	fprintf(stream, "\train.rate: %.2f\n", rain->rate);
	fprintf(stream, "\train.accum_hour: %.2f\n", rain->accum_hour);
	fprintf(stream, "\train.accum_24h: %.2f\n", rain->accum_24h);
	fprintf(stream, "\train.accum_2007: %.2f\n", rain->accum_2007);
}


static void
log_uvi(wmr_uvi *uvi, FILE *stream) {
	fprintf(stream, "\tuvi.index: %u\n", uvi->index);
}


static void
log_baro(wmr_baro *baro, FILE *stream) {
	fprintf(stream, "\tbaro.pressure: %u\n", baro->pressure);
	fprintf(stream, "\tbaro.alt_pressure: %u\n", baro->alt_pressure);
	fprintf(stream, "\tbaro.forecast: %s\n", baro->forecast);
}


static void
log_temp(wmr_temp *temp, FILE *stream) {
	uint id = temp->sensor_id;

	fprintf(stream, "\ttemp.%u.humidity: %u\n", id, temp->humidity);
	fprintf(stream, "\ttemp.%u.heat_index: %u\n", id, temp->heat_index);
	fprintf(stream, "\ttemp.%u.temp: %.1f\n", id, temp->temp);
	fprintf(stream, "\ttemp.%u.dew_point: %.1f\n", id, temp->dew_point);
}


static void
log_status(wmr_status *status, FILE *stream) {
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
log_to_file(wmr_reading *reading, FILE *stream) {
	fprintf(stream, "reading %li {\n", reading->time);

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

	fprintf(stream, "}\n\n");
}



