/*
 * loggers/server.c:
 * Server component of the WMR daemon
 *
 * This software may be freely used and distributed according to the terms
 * of the GNU GPL version 2 or 3. See LICENSE for more information.
 *
 * Copyright (c) 2015 David Čepelík <cepelik@gymlit.cz>
 */


#include "common.h"
#include "serialize.h"
#include "server.h"
#include "strbuf.h"

#include <err.h>
#include <netdb.h>
#include <netinet/in.h>
#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>


#define	ARRAY_ELEM		unsigned char
#define	ARRAY_PREFIX(x)		byte_##x
#include "array.h"

#define	DEFAULT_PORT		20892


static void
mainloop(wmr_server *srv)
{
	int fd;

	DEBUG_MSG("%s", "Entering server main loop");
	for (;;) {
		/* POSIX.1: accept is a cancellation point */
		if ((fd = accept(srv->fd, NULL, 0)) == -1)
			err(1, "accept");

		DEBUG_MSG("Client accepted, fd = %u", fd);

		struct byte_array data;
		byte_array_init(&data);

		serialize_data(&data, &srv->wmr->latest);

		if (write(fd, data.elems, data.size) != data.size) {
			DEBUG_MSG("Cannot send %zu bytes of data over network",
				data.size);
		}

		(void) close(fd);
		DEBUG_MSG("%s", "Client socket closed");
	}
}


static void
cleanup(wmr_server *srv)
{
	if (srv->fd >= 0) {
		DEBUG_MSG("%s", "Closing server socket");
		(void) close(srv->fd);
	}
}


static void *
mainloop_pthread(void *x)
{
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
server_init(wmr_server *srv, wmr200 *wmr)
{
	srv->wmr = wmr;
	srv->fd = srv->thread_id = -1;
}


int
server_start(wmr_server *srv)
{
	struct addrinfo *head, *cur;
	struct addrinfo hints; 
	int port = DEFAULT_PORT;
	char portstr[6];
	int optval = 1;
	int ret;

	memset(&hints, 0, sizeof (hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;

	snprintf(portstr, sizeof(portstr), "%u", port);

	if ((ret = getaddrinfo(NULL, portstr, &hints, &head)) != 0) {
		DEBUG_MSG("getaddrinfo: %s\n", gai_strerror(ret));
		return (-1);
	}

	for (cur = head; cur != NULL; cur = cur->ai_next) {
		srv->fd = socket(cur->ai_family,
			cur->ai_socktype, cur->ai_protocol);

		if (srv->fd == -1)
			continue;

		/* attempt to set SO_REUSEADDR, if it fails, proceed anyway */
		(void) setsockopt(srv->fd, SOL_SOCKET, SO_REUSEADDR,
			&optval, sizeof (optval));

		/* break out if we're bound */
		if (bind(srv->fd, cur->ai_addr, cur->ai_addrlen) == 0)
			break;

		(void) close(srv->fd);
	}

	freeaddrinfo(head);

	/* if cur == NULL, we are not bound to any address  */
	if (cur == NULL) {
		DEBUG_MSG("%s", "Cannot setup server socket");
		return (-1);
	}

	if (listen(srv->fd, SOMAXCONN) == -1) {
		DEBUG_MSG("%s", "Cannot start listening");
		return (-1);
	}

	DEBUG_MSG("Server start sucessfull, descriptor is %d", srv->fd);

	if (pthread_create(&srv->thread_id, NULL, mainloop_pthread, srv) != 0) {
		DEBUG_MSG("%s", "Cannot start server main loop thread");
		return (-1);
	}

	return (0);
}


void
server_stop(wmr_server *srv)
{
	pthread_cancel(srv->thread_id);
	pthread_join(srv->thread_id, NULL);
}
