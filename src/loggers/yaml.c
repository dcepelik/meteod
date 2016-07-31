/*
 * loggers/yaml.c:
 * Log readings serialized in YAML to a file or stream
 *
 * This software may be freely used and distributed according to the terms
 * of the GNU GPL version 2 or 3. See LICENSE for more information.
 *
 * Copyright (c) 2015 David Čepelík <cepelik@gymlit.cz>
 */


#include "yaml.h"
#include "wmrdata.h"

#include <stdio.h>


/*
 * loggers for individual data readings
 */


static void
log_wind(wmr_wind *wind, FILE *stream)
{
	fprintf(stream, "dir: %s\n", wind->dir);
	fprintf(stream, "gust_speed: %.2f\n", wind->gust_speed);
	fprintf(stream, "avg_speed: %.2f\n", wind->avg_speed);
	fprintf(stream, "chill: %.1f\n", wind->chill);
}


static void
log_rain(wmr_rain *rain, FILE *stream)
{
	fprintf(stream, "rate: %.2f\n", rain->rate);
	fprintf(stream, "accum_hour: %.2f\n", rain->accum_hour);
	fprintf(stream, "accum_24h: %.2f\n", rain->accum_24h);
	fprintf(stream, "accum_2007: %.2f\n", rain->accum_2007);
}


static void
log_uvi(wmr_uvi *uvi, FILE *stream)
{
	fprintf(stream, "index: %u\n", uvi->index);
}


static void
log_baro(wmr_baro *baro, FILE *stream)
{
	fprintf(stream, "pressure: %u\n", baro->pressure);
	fprintf(stream, "alt_pressure: %u\n", baro->alt_pressure);
	fprintf(stream, "forecast: %s\n", baro->forecast);
}


static void
log_temp(wmr_temp *temp, FILE *stream)
{
	fprintf(stream, "humidity: %u\n", temp->humidity);
	fprintf(stream, "heat_index: %u\n", temp->heat_index);
	fprintf(stream, "temp: %.1f\n", temp->temp);
	fprintf(stream, "dew_point: %.1f\n", temp->dew_point);
}


static void
log_status(wmr_status *status, FILE *stream)
{
	fprintf(stream, "wind_bat: %s\n", status->wind_bat);
	fprintf(stream, "temp_bat: %s\n", status->temp_bat);
	fprintf(stream, "rain_bat: %s\n", status->rain_bat);
	fprintf(stream, "uv_bat: %s\n", status->uv_bat);

	fprintf(stream, "wind_sensor: %s\n", status->wind_sensor);
	fprintf(stream, "temp_sensor: %s\n", status->temp_sensor);
	fprintf(stream, "rain_sensor: %s\n", status->rain_sensor);
	fprintf(stream, "uv_sensor: %s\n", status->uv_sensor);
}


static void
log_meta(wmr_meta *meta, FILE *stream)
{
	fprintf(stream, "num_packets: %u\n", meta->num_packets);
	fprintf(stream, "num_failed: %u\n", meta->num_failed);
	fprintf(stream, "num_frames: %u\n", meta->num_frames);
	fprintf(stream, "error_rate: %.1f\n", meta->error_rate);
	fprintf(stream, "num_bytes: %li\n", meta->num_bytes);
	fprintf(stream, "latest_packet: %li\n", meta->latest_packet);
	fprintf(stream, "uptime: %li\n", meta->uptime);
}


static void
log_reading(wmr_reading *reading, FILE *stream)
{
	fprintf(stream, "---\n\n");
	fprintf(stream, "sensor: %s\n", wmr_sensor_name(reading));
	fprintf(stream, "time: %li\n", reading->time);

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

	fprintf(stream, "\n");
	fflush(stream);
}


/*
 * public interface
 */


void
yaml_push_reading(wmr_reading *reading, void *arg)
{
	FILE *stream = (FILE *)arg;
	log_reading(reading, stream);
}
