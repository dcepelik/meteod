/*
 * server.c:
 * Server component of the WMR daemon
 *
 * This software may be freely used and distributed according to the terms
 * of the GNU GPL version 2 or 3. See LICENSE for more information.
 *
 * Copyright (c) 2015 David Čepelík <cepelik@gymlit.cz>
 */


#include "server.h"
#include "strbuf.h"


void
log_to_server(wmr_server *srv, wmr_reading *reading) {
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
	}
}


char *
get_response(wmr_server *server, char *query) {
	strbuf buf;
	strbuf_init(&buf);

	

	strbuf_free(&buf);
}


void
server_start(wmr_server *server) {
	// wait for connections
	// when a connection is accepted, go ahead and process the query
	// go again, baby
}
