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
update_if_newer(wmr_reading *old, wmr_reading *new) {
	if (new->time >= old->time) {
		*old = *new;
	}
}


static void
log_reading(wmr_server *srv, wmr_reading *reading) {
	/* TODO add "if-newer" checks */

	switch (reading->type) {
	case WMR_WIND:
		update_if_newer(&srv->data.wind, reading);
		break;

	case WMR_RAIN:
		update_if_newer(&srv->data.rain, reading);
		break;

	case WMR_UVI:
		update_if_newer(&srv->data.uvi, reading);
		break;

	case WMR_BARO:
		update_if_newer(&srv->data.baro, reading);
		break;

	case WMR_TEMP:
		update_if_newer(&srv->data.temp[reading->temp.sensor_id],
			reading);
		break;

	case WMR_STATUS:
		update_if_newer(&srv->data.status, reading);
		break;

	case WMR_META:
		update_if_newer(&srv->data.meta, reading);
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
