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
	fprintf(stream, "\tdir: %s\n", wind->dir);
	fprintf(stream, "\tgust_speed: %.2f\n", wind->gust_speed);
	fprintf(stream, "\tavg_speed: %.2f\n", wind->avg_speed);
	fprintf(stream, "\tchill: %.1f\n", wind->chill);
	fprintf(stream, "}\n\n");
}


static void
log_rain(wmr_rain *rain, FILE *stream) {
	fprintf(stream, "rain %li {\n", rain->time);
	fprintf(stream, "\trate: %.2f\n", rain->rate);
	fprintf(stream, "\taccum_hour: %.2f\n", rain->accum_hour);
	fprintf(stream, "\taccum_24h: %.2f\n", rain->accum_24h);
	fprintf(stream, "\taccum_2007: %.2f\n", rain->accum_2007);
	fprintf(stream, "}\n\n");
}


static void
log_uvi(wmr_uvi *uvi, FILE *stream) {
	fprintf(stream, "uvi %li {\n", uvi->time);
	fprintf(stream, "\tindex: %u\n", uvi->index);
	fprintf(stream, "}\n\n");
}


static void
log_baro(wmr_baro *baro, FILE *stream) {
	fprintf(stream, "baro %li {\n", baro->time);
	fprintf(stream, "\tpressure: %u\n", baro->pressure);
	fprintf(stream, "\talt_pressure: %u\n", baro->alt_pressure);
	fprintf(stream, "\tforecast: %s\n", baro->forecast);
	fprintf(stream, "}\n\n");
}


static void
log_temp(wmr_temp *temp, FILE *stream) {
	fprintf(stream, "temp%u %li {\n", temp->sensor_id, temp->time);
	fprintf(stream, "\thumidity: %u\n", temp->humidity);
	fprintf(stream, "\theat_index: %u\n", temp->heat_index);
	fprintf(stream, "\ttemp: %.1f\n", temp->temp);
	fprintf(stream, "\tdew_point: %.1f\n", temp->dew_point);
	fprintf(stream, "}\n\n");
}


static void
log_status(wmr_status *status, FILE *stream) {
	fprintf(stream, "status %li {\n", status->time);

	fprintf(stream, "\twind_bat: %s\n", status->wind_bat);
	fprintf(stream, "\ttemp_bat: %s\n", status->temp_bat);
	fprintf(stream, "\train_bat: %s\n", status->rain_bat);
	fprintf(stream, "\tuv_bat: %s\n", status->uv_bat);

	fprintf(stream, "\twind_sensor: %s\n", status->wind_sensor);
	fprintf(stream, "\ttemp_sensor: %s\n", status->temp_sensor);
	fprintf(stream, "\train_sensor: %s\n", status->rain_sensor);
	fprintf(stream, "\tuv_sensor: %s\n", status->uv_sensor);

	fprintf(stream, "\trtc_signal_level: %s\n", status->rtc_signal_level);

	fprintf(stream, "}\n\n");
}


static void
log_meta(wmr_meta *meta, FILE *stream) {
	fprintf(stream, "meta %li {\n", meta->time);

	fprintf(stream, "num_packets: %u", meta->num_packets);
	fprintf(stream, "num_failed: %u", meta->num_failed);
	fprintf(stream, "num_frames: %u", meta->num_frames);
	fprintf(stream, "error_rate: %.1f", meta->error_rate);
	fprintf(stream, "num_bytes: %li", meta->num_bytes);
	fprintf(stream, "latest_packet: %li", meta->latest_packet);

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

	case WMR_META:
		log_meta(&reading->meta, stream);
		break;
	}
}



