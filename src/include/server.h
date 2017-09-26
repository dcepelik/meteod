/*
 * server.h:
 * Server component of the WMR daemon
 *
 * This software may be freely used and distributed according to the terms
 * of the GNU GPL version 2 or 3. See LICENSE for more information.
 *
 * Copyright (c) 2015 David Čepelík <cepelik@gymlit.cz>
 */

#include <pthread.h>

#include "wmr200.h"


struct wmr_server {
	struct wmr200 *wmr;	/* the device we serve data for */
	int fd;			/* server socket descriptor */
	pthread_t thread_id;	/* server thread ID */
};


void server_init(struct wmr_server *srv, struct wmr200 *wmr);
int server_start(struct wmr_server *srv);
void server_stop(struct wmr_server *srv);
