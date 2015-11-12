/*
 * wmrd.c
 * WMR logging daemon
 *
 * This software may be freely used and distributed according to the terms
 * of the GNU GPL version 2 or 3. See LICENSE for more information.
 *
 * Copyright (c) 2015 David Čepelík <cepelik@gymlit.cz>
 */


#include <stdio.h>
#include <pthread.h>
#include <signal.h>
#include <string.h>
#include <unistd.h>

#include "wmr200.h"
#include "loggers/file.h"
#include "loggers/rrd.h"
#include "loggers/server.h"


wmr200 *wmr;


static void
shutdown(int signum) {
	DEBUG_MSG("Caught signal %d (%s)", signum, strsignal(signum));
}


static void
handler(wmr_reading *reading) {
	log_to_file(reading, stdout);
	log_to_rrd(reading, "/home/david/gymlit/tools/meteo/temp.rrd");
	// log to server
}


int
main(int argc, const char *argv[]) {
	sigset_t set, oldset;
	pthread_t comm_thread, server_thread;
	wmr_server srv;

	server_init(&srv);

	/* mask SIGINT and SIGTERM to make sure this thread handles them */
	sigemptyset(&set);
	sigaddset(&set, SIGINT);
	sigaddset(&set, SIGTERM);
	pthread_sigmask(SIG_BLOCK, &set, &oldset);

	/* start the logger daemon and socket server threads */
	// pthread_create(&comm_thread, NULL, wmr_main_loop, wmr);
	pthread_create(&server_thread, NULL, server_main_loop_pthread, &srv);

	/* install of SIGTERM and SIGINT signals */
	struct sigaction sa;
	sa.sa_handler = shutdown;
	sa.sa_flags = 0;
	sigaction(SIGTERM, &sa, NULL);
	sigaction(SIGINT, &sa, NULL);

	/* restore original sigmask for this thread only */
	pthread_sigmask(SIG_SETMASK, &oldset, NULL);

	/* wait for SIGINT/SIGKILL to arrive, then shutdown */
	pause();

	//pthread_cancel(comm_thread);
	pthread_cancel(server_thread);

	//pthread_join(comm_thread, NULL);
	pthread_join(server_thread, NULL);

	fprintf(stderr, "\n\n%s: graceful daemon termination\n", argv[0]);
	return (EXIT_SUCCESS);
}



