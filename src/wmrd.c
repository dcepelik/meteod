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


static void
shutdown(int signum) {
	DEBUG_MSG("Caught signal %d (%s)", signum, strsignal(signum));
}


static void
handler(wmr_reading *reading) {
	log_to_file(reading, stdout);
	log_to_rrd(reading, "/home/david/gymlit/tools/meteo/temp.rrd");
}


int
main(int argc, const char *argv[]) {
	sigset_t set, oldset;
	wmr_server srv;

	/* mask SIGINT and SIGTERM away from spawned processes */
	sigemptyset(&set);
	sigaddset(&set, SIGINT);
	sigaddset(&set, SIGTERM);
	pthread_sigmask(SIG_BLOCK, &set, &oldset);

	server_init(&srv);
	server_start(&srv);

	/* install signal handlers */
	struct sigaction sa;
	sa.sa_handler = shutdown;
	sa.sa_flags = 0;
	sigaction(SIGTERM, &sa, NULL);
	sigaction(SIGINT, &sa, NULL);

	/* restore original sigmask for this thread only */
	pthread_sigmask(SIG_SETMASK, &oldset, NULL);

	/* wait for SIGINT/SIGKILL to arrive, then shutdown */
	pause();

	// pthread_cancel(comm_thread);
	server_stop(&srv);

	// pthread_join(comm_thread, NULL);

	fprintf(stderr, "\n\n%s: graceful termination\n", argv[0]);
	return (EXIT_SUCCESS);
}
