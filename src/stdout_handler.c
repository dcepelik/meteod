#include "stdout_handler.h"
#include "wmr200.h"


static void
handle_wind(struct wmr_wind_reading *wind) {
	printf("\twind.dir: %s\n", wind->dir);
	printf("\twind.gust_speed: %.2f\n", wind->gust_speed);
	printf("\twind.avg_speed: %.2f\n", wind->avg_speed);
	printf("\twind.chill: %.1f\n", wind->chill);
}


static void
handle_rain(struct wmr_rain_reading *rain) {
	printf("\train.rate: %.2f\n", rain->rate);
	printf("\train.accum_hour: %.2f\n", rain->accum_hour);
	printf("\train.accum_24h: %.2f\n", rain->accum_24h);
	printf("\train.accum_2007: %.2f\n", rain->accum_2007);
}


static void
handle_uvi(struct wmr_uvi_reading *uvi) {
	printf("\tuvi.index: %u\n", uvi->index);
}


static void
handle_baro(struct wmr_baro_reading *baro) {
	printf("\tbaro.pressure: %u\n", baro->pressure);
	printf("\tbaro.alt_pressure: %u\n", baro->alt_pressure);
	printf("\tbaro.forecast: %s\n", baro->forecast);
}


static void
handle_temp(struct wmr_temp_reading *temp) {
	uint id = temp->sensor_id;

	printf("\ttemp.%u.humidity: %u\n", id, temp->humidity);
	printf("\ttemp.%u.heat_index: %u\n", id, temp->heat_index);
	printf("\ttemp.%u.temp: %.1f\n", id, temp->temp);
	printf("\ttemp.%u.dew_point: %.1f\n", id, temp->dew_point);
}


static void
handle_status(struct wmr_status_reading *status) {
	printf("\tstatus.wind_bat: %s\n", status->wind_bat);
	printf("\tstatus.temp_bat: %s\n", status->temp_bat);
	printf("\tstatus.rain_bat: %s\n", status->rain_bat);
	printf("\tstatus.uv_bat: %s\n", status->uv_bat);

	printf("\tstatus.wind_sensor: %s\n", status->wind_sensor);
	printf("\tstatus.temp_sensor: %s\n", status->temp_sensor);
	printf("\tstatus.rain_sensor: %s\n", status->rain_sensor);
	printf("\tstatus.uv_sensor: %s\n", status->uv_sensor);

	printf("\tstatus.rtc_signal_level: %s\n", status->rtc_signal_level);
}


void
stdout_handler(struct wmr_reading *reading) {
	printf("reading %li {\n", reading->time);

	switch (reading->type) {
	case WIND_DATA:
		handle_wind(&reading->wind);
		break;

	case RAIN_DATA:
		handle_rain(&reading->rain);
		break;

	case UVI_DATA:
		handle_uvi(&reading->uvi);
		break;

	case BARO_DATA:
		handle_baro(&reading->baro);
		break;

	case TEMP_DATA:
		handle_temp(&reading->temp);
		break;

	case STATUS_DATA:
		handle_status(&reading->status);
		break;
	}

	printf("}\n\n");
}



