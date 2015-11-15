/*
 * loggers/server.c:
 * Make readings available to the server
 *
 * This software may be freely used and distributed according to the terms
 * of the GNU GPL version 2 or 3. See LICENSE for more information.
 *
 * Copyright (c) 2015 David Čepelík <cepelik@gymlit.cz>
 */

#include "../server.h" /* TODO */


static void
log_reading(wmr_server *srv, wmr_reading *reading) {
	switch (reading->type) {
	case WMR_WIND:
		srv->data.wind = reading->wind;
		break;

	case WMR_RAIN:
		srv->data.rain = reading->rain;
		break;

	case WMR_UVI:
		srv->data.uvi = reading->uvi;
		break;

	case WMR_BARO:
		srv->data.baro = reading->baro;
		break;

	case WMR_TEMP:
		srv->data.temp[reading->temp.sensor_id] = reading->temp;
		break;

	case WMR_STATUS:
		srv->data.status = reading->status;
		break;

	case WMR_META:
		srv->data.meta = reading->meta;
		break;
	}
}


/*
 * public interface
 */


void
server_push_reading(wmr_reading *reading, void *arg) {
	wmr_server *srv = (wmr_server *)arg;
	log_reading(srv, reading);
}
