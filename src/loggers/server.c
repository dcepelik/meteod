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
#include "time.h"


#define ARRAY_ELEM		unsigned char
#define ARRAY_PREFIX(x)		byte_##x
#include "array.h"


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


void
send_latest_data(wmr_server *server) {
}


static void
main_loop() {
	
}


void
server_start(wmr_server *server) {
	int server_fd, client_fd;
	int port = 20892;
	int optval = 1;

	struct sockaddr_in in = {
		.sin_family = AF_INET,
		.sin_port = htons(port),
		.sin_addr.s_addr = htonl(INADDR_ANY),
	};

	if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
		err(1, "socket");

	DEBUG_MSG("Socket is open, server_fd = %u", server_fd);

	if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval)) == -1)
		err(1, "setsockopt");

	DEBUG_MSG("%s", "Set SO_REUSEADDR on server socket");

	if (bind(server_fd, (struct sockaddr *) &in, sizeof(in)) == -1)
		err(1, "bind");

	DEBUG_MSG("Bound to port %u", port);

	if (listen(server_fd, SOMAXCONN) == -1)
		err(1, "listen");

	DEBUG_MSG("%s", "Server is listening for incoming connections");

	for (;;) {
		/* POSIX.1: accept is cancellation point */
		if ((client_fd = accept(server_fd, NULL, 0)) == -1)
			err(1, "accept");

		DEBUG_MSG("Client accepted, client_fd = %u", client_fd);

		/*
		if (write(client_fd, arr.elems, arr.size) != arr.size) {
			DEBUG_MSG("Cannot send %zu bytes over nework", arr.size);
		}
		*/

		(void)close(client_fd);
	}

	(void)close(server_fd);
}



