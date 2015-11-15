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
#include <time.h>
#include <pthread.h>

#define	ARRAY_ELEM		unsigned char
#define	ARRAY_PREFIX(x)		byte_##x
#include "array.h"


static void
cleanup(wmr_server *srv) {
	if (srv->fd >= 0) {
		DEBUG_MSG("%s", "Closing server socket");
		(void) close(srv->fd);
	}
}


static void
mainloop(wmr_server *srv) {
	int fd;

	DEBUG_MSG("%s", "Entering server main loop");
	for (;;) {
		/* POSIX.1: accept is a cancellation point */
		if ((fd = accept(srv->fd, NULL, 0)) == -1)
			err(1, "accept");

		DEBUG_MSG("Client accepted, socket descriptor is %u", fd);

		struct byte_array data;
		byte_array_init(&data);

		serialize_data(&data, &srv->data);

		if (write(fd, data.elems, data.size) != data.size) {
			fprintf(stderr, "Cannot send %zu bytes of data\n",
				data.size);
		}

		(void) close(fd);
		DEBUG_MSG("%s", "Client socket closed");
	}
}


static void *
mainloop_pthread(void *x) {
	wmr_server *srv = (wmr_server *)x;

	pthread_cleanup_push(cleanup, srv);
	mainloop(srv);
	pthread_cleanup_pop(0);

	return (NULL);
}


/*
 * public interface
 */


void
server_init(wmr_server *srv) {
	memset(&(srv->data), 0, sizeof (srv->data)); /* will be sent over net */
	srv->fd = srv->thread_id = -1;
}


int
server_start(wmr_server *srv) {
	int port = 20892;
	int optval = 1;
	int ret;

	struct sockaddr_in in = {
		.sin_family = AF_INET,
		.sin_port = htons(port),
		.sin_addr.s_addr = htonl(INADDR_ANY),
	};

	if ((srv->fd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
		DEBUG_MSG("%s", "Cannot open server socket");
		return (-1);
	}

	ret = setsockopt(srv->fd, SOL_SOCKET, SO_REUSEADDR,
		&optval, sizeof (optval));

	if (ret == -1) {
		DEBUG_MSG("%s", "Cannot set SO_REUSEADDR on server socket");
		return (-1);
	}

	if (bind(srv->fd, (struct sockaddr *)&in, sizeof (in)) == -1) {
		DEBUG_MSG("Cannot bind server socket to port %d", port);
		return (-1);
	}

	if (listen(srv->fd, SOMAXCONN) == -1) {
		DEBUG_MSG("%s", "Cannot start listening");
		return (-1);
	}

	DEBUG_MSG("Server start sucessfull, descriptor is %d", srv->fd);

	if (pthread_create(&srv->thread_id, NULL, mainloop_pthread, srv) != 0) {
		DEBUG_MSG("%s", "Cannot start main server loop");
		return (-1);
	}

	return (0);
}


void
server_stop(wmr_server *srv) {
	pthread_cancel(srv->thread_id);
	pthread_join(srv->thread_id, NULL);
}
