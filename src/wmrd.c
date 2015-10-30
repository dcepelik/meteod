#include <stdio.h>
#include <signal.h>

#include "wmr200.h"
#include "macros.h"


struct wmr200 *wmr;


static void
cleanup(int signum) {
	wmr_close(wmr);
	wmr_end();

	printf("\n\nCaught signal %i, will exit\n", signum);
	exit(0);
}


void
data_handler(struct wmr_reading *reading) {
	uint id;

	switch (reading->type) {
	case WIND_DATA:
		printf("\twind.dir: %s\n", reading->wind.dir);
		printf("\twind.gust_speed: %.2f\n", reading->wind.gust_speed);
		printf("\twind.avg_speed: %.2f\n", reading->wind.avg_speed);
		printf("\twind.chill: %.1f\n", reading->wind.chill);
		break;

	case RAIN_DATA:
		printf("\train.rate: %.2f\n", reading->rain.rate);
		printf("\train.accum_hour: %.2f\n", reading->rain.accum_hour);
		printf("\train.accum_24h: %.2f\n", reading->rain.accum_24h);
		printf("\train.accum_2007: %.2f\n", reading->rain.accum_2007);
		break;

	case UVI_DATA:
		printf("\tuvi.index: %u\n", reading->uvi.index);
		break;

	case BARO_DATA:
		printf("\tbaro.pressure: %u\n", reading->baro.pressure);
		printf("\tbaro.alt_pressure: %u\n", reading->baro.alt_pressure);
		printf("\tbaro.forecast: %s\n", reading->baro.forecast);
		break;

	case TEMP_DATA:
		id = reading->temp.sensor_id;

		printf("\ttemp.%u.humidity: %u\n", id, reading->temp.humidity);
		printf("\ttemp.%u.heat_index: %u\n", id, reading->temp.heat_index);
		printf("\ttemp.%u.temp: %.1f\n", id, reading->temp.temp);
		printf("\ttemp.%u.dew_point: %.1f\n", id, reading->temp.dew_point);
		break;

	case STATUS_DATA:
		printf("\tstatus.wind.bat: %s\n", reading->status.wind_bat);
		printf("\tstatus.temp.bat: %s\n", reading->status.temp_bat);
		printf("\tstatus.rain.bat: %s\n", reading->status.rain_bat);
		printf("\tstatus.uv.bat: %s\n", reading->status.uv_bat);

		printf("\tstatus.wind.sensor: %s\n", reading->status.wind_sensor);
		printf("\tstatus.temp.sensor: %s\n", reading->status.temp_sensor);
		printf("\tstatus.rain.sensor: %s\n", reading->status.rain_sensor);
		printf("\tstatus.uv.sensor: %s\n", reading->status.uv_sensor);

		printf("\tstatus.rtc.signal: %s\n", reading->status.rtc_signal_level);
	}
}


int
main(int argc, const char *argv[]) {
	struct sigaction sa;
	sa.sa_handler = cleanup;
	sigaction(SIGTERM, &sa, NULL); // TODO
	sigaction(SIGINT, &sa, NULL); // TODO

	wmr_init();

	wmr = wmr_open();
	if (!wmr) {
		fprintf(stderr, "wmr_connect(): no WMR200 handle returned\n");
		return (1);
	}

	wmr_set_handler(wmr, data_handler);
	wmr_set_handler(wmr, data_handler);

	wmr_main_loop(wmr);

	wmr_close(wmr);
	wmr_end();

	return (0);
}
