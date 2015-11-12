/*
 * loggers/server.c:
 * Server component of the WMR daemon
 *
 * This software may be freely used and distributed according to the terms
 * of the GNU GPL version 2 or 3. See LICENSE for more information.
 *
 * Copyright (c) 2015 David Čepelík <cepelik@gymlit.cz>
 */


#include "server.h"
#include "strbuf.h"
#include "serialize.h"
#include "common.h"

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <err.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <rpc/xdr.h>
#include <time.h>
#include <pthread.h>


#define ARRAY_ELEM		unsigned char
#define ARRAY_PREFIX(x)		byte_##x
#include "array.h"


static void
mainloop(wmr_server *srv) {
	int fd;
	int port = 20892;
	int optval = 1;

	struct sockaddr_in in = {
		.sin_family = AF_INET,
		.sin_port = htons(port),
		.sin_addr.s_addr = htonl(INADDR_ANY),
	};

	if ((srv->fd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
		err(1, "socket");

	DEBUG_MSG("Server socket is open, socket fd is %u", srv->fd);

	if (setsockopt(srv->fd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval)) == -1)
		err(1, "setsockopt");

	DEBUG_MSG("%s", "Set SO_REUSEADDR on server socket");

	if (bind(srv->fd, (struct sockaddr *) &in, sizeof(in)) == -1)
		err(1, "bind");

	DEBUG_MSG("Bound to port %u", port);

	if (listen(srv->fd, SOMAXCONN) == -1)
		err(1, "listen");

	DEBUG_MSG("%s", "Server is listening for incoming connections");

	for (;;) {
		/* POSIX.1: accept is a cancellation point */
		if ((fd = accept(srv->fd, NULL, 0)) == -1)
			err(1, "accept");

		DEBUG_MSG("Client accepted, socket fd is %u", fd);

		/*
		if (write(fd, arr.elems, arr.size) != arr.size) {
			DEBUG_MSG("Cannot send %zu bytes over nework", arr.size);
		}
		*/

		(void)close(fd);
	}
}


static void
cleanup(wmr_server *srv) {
	if (srv->fd >= 0) {
		DEBUG_MSG("%s", "Terminating server socket");
		(void)close(srv->fd);
	}
}


/********************** logger api **********************/


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


/********************** public interface **********************/


void
server_init(wmr_server *srv) {
	memset(&srv->data, 0, sizeof(srv->data)); /* will be sent over net */
	srv->fd = -1;
}


void *
server_pthread_mainloop(void *x) {
	wmr_server *srv = (wmr_server *)x;

	pthread_cleanup_push(cleanup, srv);
	mainloop(srv);
	pthread_cleanup_pop(0);

	return NULL;
}



