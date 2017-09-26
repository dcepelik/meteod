/*
 * Make data available over TCP/IP.
 */

#include "log.h"
#include "server.h"

#include <err.h>
#include <netdb.h>
#include <netinet/in.h>
#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

#define	DEFAULT_PORT		20892

static void mainloop(struct wmr_server *srv)
{
	int fd;

	log_info("%s", "Entering server main loop");
	while (1) {
		/* POSIX.1: accept is a cancellation point */
		if ((fd = accept(srv->fd, NULL, 0)) == -1)
			err(1, "accept"); /* TODO don't use err */

		log_debug("Client accepted, fd = %u", fd);

		/* TODO send data */

		close(fd);
		log_debug("%s", "Client socket closed");
	}
}

static void cleanup(void *arg)
{
	struct wmr_server *srv = (struct wmr_server *)arg;

	if (srv->fd >= 0) {
		log_info("%s", "Closing server socket");
		close(srv->fd);
	}
}

static void *mainloop_pthread(void *x)
{
	struct wmr_server *srv = (struct wmr_server *)x;

	pthread_cleanup_push(cleanup, srv);
	mainloop(srv);
	pthread_cleanup_pop(0);

	return (NULL);
}

void server_init(struct wmr_server *srv, struct wmr200 *wmr)
{
	srv->wmr = wmr;
	srv->fd = srv->thread_id = -1;
}

int server_start(struct wmr_server *srv)
{
	struct addrinfo *ai_head, *ai_cur;
	struct addrinfo ai_hints; 
	int port = DEFAULT_PORT;
	char portstr[6];
	int optval = 1;
	int ret;

	memset(&ai_hints, 0, sizeof(ai_hints));
	ai_hints.ai_family = AF_UNSPEC;
	ai_hints.ai_socktype = SOCK_STREAM;
	ai_hints.ai_flags = AI_PASSIVE;

	snprintf(portstr, sizeof(portstr), "%u", port);

	if ((ret = getaddrinfo(NULL, portstr, &ai_hints, &ai_head)) != 0) {
		log_error("getaddrinfo: %s\n", gai_strerror(ret));
		return -1;
	}

	for (ai_cur = ai_head; ai_cur != NULL; ai_cur = ai_cur->ai_next) {
		srv->fd = socket(ai_cur->ai_family, ai_cur->ai_socktype,
			ai_cur->ai_protocol);

		if (srv->fd == -1)
			continue;

		setsockopt(srv->fd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));
		if (bind(srv->fd, ai_cur->ai_addr, ai_cur->ai_addrlen) == 0)
			break;

		close(srv->fd);
	}

	freeaddrinfo(ai_head);

	/* if ai_cur == NULL, we are not bound to any address  */
	if (ai_cur == NULL) {
		log_error("%s", "Cannot bind to any address");
		return -1;
	}

	if (listen(srv->fd, SOMAXCONN) == -1) {
		log_error("listen: %s", "Cannot start listening");
		return -1;
	}

	log_info("Server start sucessfull, descriptor is %d", srv->fd);

	if (pthread_create(&srv->thread_id, NULL, mainloop_pthread, srv) != 0) {
		log_error("%s", "Cannot start server main loop thread");
		return -1;
	}

	return 0;
}

void server_stop(struct wmr_server *srv)
{
	pthread_cancel(srv->thread_id);
	pthread_join(srv->thread_id, NULL);
}
