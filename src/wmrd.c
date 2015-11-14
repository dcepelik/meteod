/*
 * wmrd.c:
 * WMR logging daemon
 *
 * This software may be freely used and distributed according to the terms
 * of the GNU GPL version 2 or 3. See LICENSE for more information.
 *
 * Copyright (c) 2015 David Čepelík <cepelik@gymlit.cz>
 */


#include "wmr200.h"
#include "daemon.h"
#include "loggers/file.h"
#include "loggers/rrd.h"
#include "loggers/server.h"

#include <stdio.h>
#include <pthread.h>
#include <signal.h>
#include <string.h>
#include <unistd.h>
#include <syslog.h>
#include <getopt.h>


#define	ARRAY_ELEM		unsigned char
#define	ARRAY_PREFIX(x)		byte_##x
#include "array.h"
#include "serialize.h"


wmr_server srv;


static struct option longopts[] = {
	{ "daemon",	no_argument,	NULL,	'd' }};


static void
signal_handler(int signum) {
	DEBUG_MSG("Caught signal %d (%s)", signum, strsignal(signum));
}


float deserialize_float(struct byte_array * arr);
void serialize_float(struct byte_array * arr, float f);


int
main(int argc, char *argv[]) {
	/*float f = 2/3.0;

	struct byte_array arr;
	byte_array_init(&arr);

	serialize_float(&arr, f);

	printf("%f\n", deserialize_float(&arr));
	
	return 0;
	*/

	sigset_t set, oldset;
	wmr200 *wmr;
	int c, dflag = 0;

	while ((c = getopt_long(argc, argv, "d", longopts, NULL)) != -1) {
		switch (c) {
		case 'd':
			dflag = 1;
			break;

		default:
			fprintf(stderr, "%s: usage: %s [OPTIONS]\n",
				argv[0], argv[0]);
			fprintf(stderr,
				"\t-d, --daemonize\t\tdaemonize the process\n"
				"\t-c, --config FILE\tuse config file FILE\n"
				"\t-S, --server\t\tuse logging server\n"
				"\t-F, --file FILE\t\tlog readings to FILE\n"
				"\t-R, --rrd DIR\t\tlog to RRDs in DIR\n"
				"\n");

			exit(EXIT_FAILURE);
		}
	}

	sigemptyset(&set);
	sigaddset(&set, SIGINT);
	sigaddset(&set, SIGTERM);
	pthread_sigmask(SIG_BLOCK, &set, &oldset);

	wmr_init();

	wmr = wmr_open();
	if (wmr == NULL)
		die("wmr_open: no WMR200 handle returned\n");

	wmr_add_handler(wmr, file_push_reading, stderr);
	wmr_add_handler(wmr, rrd_push_reading, "none");
	wmr_add_handler(wmr, server_push_reading, &srv);

	if (wmr_start(wmr) != 0)
		die("wmr_start: cannot start WMR comm loop\n");

	server_init(&srv);
	if (server_start(&srv) != 0)
		die("server_start: cannot start the WMR server instance\n");


	struct sigaction sa;
	memset(&sa, 0, sizeof (sa));
	sa.sa_handler = signal_handler;
	sigaction(SIGTERM, &sa, NULL);
	sigaction(SIGINT, &sa, NULL);

	pthread_sigmask(SIG_SETMASK, &oldset, NULL);

	if (dflag) 
		daemonize(argv[0]);

	/* wait here for SIGINT/SIGTERM */
	pause();

	wmr_stop(wmr);
	wmr_close(wmr);
	wmr_end();

	server_stop(&srv);

	syslog(LOG_NOTICE, "%s: graceful termination\n", argv[0]);
	return (EXIT_SUCCESS);
}
