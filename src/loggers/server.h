/*
 * loggers/server.h:
 * Server component of the WMR daemon
 *
 * This software may be freely used and distributed according to the terms
 * of the GNU GPL version 2 or 3. See LICENSE for more information.
 *
 * Copyright (c) 2015 David Čepelík <cepelik@gymlit.cz>
 */

#ifndef SERVER_H
#define SERVER_H

#include "wmr200.h"
#include "wmrdata.h"


struct latest_data {
	wmr_wind wind;
	wmr_rain rain;
	wmr_uvi uvi;
	wmr_baro baro;
	wmr_temp temp[WMR200_MAX_TEMP_SENSORS];
	wmr_status status;
};


typedef struct {
	int fd;				/* server socket descriptor */
	pthread_t thread_id;		/* server thread ID */
	struct latest_data data;	/* latest readings of all kinds */
} wmr_server;


void
server_init(wmr_server *srv);


void
server_push_reading(wmr_server *srv, wmr_reading *reading);


int
server_start(wmr_server *srv);


void
server_stop(wmr_server *srv);


#endif
