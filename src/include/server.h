#ifndef SERVER_H
#define SERVER_H

#include "wmr200.h"
#include <pthread.h>

struct wmr_server_cfg
{
	unsigned port;		/* TCP port number */
};

/*
 * TCP/IP server execution context.
 */
struct wmr_server
{
	struct wmr200 *wmr;	/* the device we serve data for */
	int fd;			/* server socket descriptor */
	pthread_t thread_id;	/* server thread ID */
};

void server_init(struct wmr_server *srv);
void server_set_device(struct wmr_server *srv, struct wmr200 *wmr);
int server_start(struct wmr_server *srv);
void server_stop(struct wmr_server *srv);

#endif
