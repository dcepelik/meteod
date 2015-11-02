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
#include <signal.h>

#include "wmr200.h"
#include "server.h"
#include "loggers/file.h"
#include "loggers/rrd.h"
#include "loggers/server.h"


wmr200 *wmr;


static void
cleanup(int signum) {
	wmr_close(wmr);
	wmr_end();

	printf("\n\nCaught signal %i, will exit\n", signum);
	exit(0);
}


static void
handler(wmr_reading *reading) {
	log_to_file(reading, stdout);
	log_to_rrd(reading, "/home/david/gymlit/tools/meteo/temp.rrd");
	// log to server
}


int
main(int argc, const char *argv[]) {
	struct sigaction sa;
	sa.sa_handler = cleanup;
	sigaction(SIGTERM, &sa, NULL); // TODO
	sigaction(SIGINT, &sa, NULL); // TODO

	wmr_init();

	wmr = wmr_open();
	if (wmr == NULL) {
		fprintf(stderr, "wmr_connect(): no WMR200 handle returned\n");
		return (1);
	}

	wmr_set_handler(wmr, handler);

	wmr_main_loop(wmr);

	wmr_close(wmr);
	wmr_end();

	return (0);
}



