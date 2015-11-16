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
#include "server.h"
#include "loggers/yaml.h"
#include "loggers/rrd.h"
#include "loggers/server.h"

#include <stdio.h>
#include <pthread.h>
#include <signal.h>
#include <string.h>
#include <unistd.h>
#include <syslog.h>
#include <getopt.h>


int flag_daemonize = 0;
int flag_run_server = 0;
int port = 20892;

static struct option longopts[] = {
	{ "daemon",		no_argument,		NULL,		'd' },
	{ "config-file",	required_argument,	NULL,		'c' },
	{ "run-server",		no_argument,		NULL,		'S' },
	{ "port",		required_argument,	&port,		'p' },
	{ "to-file",		required_argument,	NULL,		'F' },
	{ "to-rrd",		required_argument,	NULL,		'R' },
};

static const char *optstr = "dc:Sp:F:R:";


static void
signal_handler(int signum)
{
	DEBUG_MSG("Caught signal %d (%s)", signum, strsignal(signum));
}


static void
usage(char *argv0)
{
	fprintf(stderr, "%s: usage: %s [OPTIONS]\n", argv0, argv0);
	fprintf(stderr,
		"\t-d, --daemonize\t\tdaemonize the process\n"
		"\t-c, --config FILE\tuse config file FILE\n"
		"\t-S, --server\t\tuse logging server\n"
		"\t-F, --file FILE\t\tlog readings to FILE\n"
		"\t-R, --rrd DIR\t\tlog to RRDs in DIR\n"
		"\n");
}


int
main(int argc, char *argv[])
{
	char *argv0 = argv[0];

	sigset_t set, oldset;
	wmr200 *wmr;
	wmr_server srv;
	int c;

	/* mask signals away from spawned threads */
	sigemptyset(&set);
	sigaddset(&set, SIGINT);
	sigaddset(&set, SIGTERM);
	pthread_sigmask(SIG_BLOCK, &set, &oldset);

	wmr_init();

	wmr = wmr_open();
	//wmr = malloc(sizeof(wmr200));
	if (wmr == NULL)
		die("wmr_open: no WMR200 handle returned\n");

	if (wmr_start(wmr) != 0)
		die("wmr_start: cannot start WMR comm loop\n");

	while ((c = getopt_long(argc, argv, optstr, longopts, NULL)) != -1) {
		switch (c) {
		case 'd':
			flag_daemonize = 1;
			break;

		case 'c':
			/* TODO config file? */
			break;

		case 'S':
			server_init(&srv, wmr);
			if (server_start(&srv) != 0)
				die("server_start: cannot start the WMR server "
					"instance\n");

			wmr_add_handler(wmr, server_push_reading, &srv);
			break;

		case 'F':
			/* TODO take argv into account */
			wmr_add_handler(wmr, yaml_push_reading, stderr);
			break;

		case 'R':
			/* TODO take argv into account */
			wmr_add_handler(wmr, rrd_push_reading, "none");
			break;

		default:
			usage(argv0);
			exit(EXIT_FAILURE);
		}
	}

	struct sigaction sa;
	memset(&sa, 0, sizeof (sa));
	sa.sa_handler = signal_handler;
	sigaction(SIGTERM, &sa, NULL);
	sigaction(SIGINT, &sa, NULL);

	pthread_sigmask(SIG_SETMASK, &oldset, NULL);

	if (flag_daemonize) 
		daemonize(argv0);


	pause(); /* wait here for SIGINT or SIGTERM to arrive */

	wmr_stop(wmr);
	wmr_close(wmr);
	wmr_end();

	server_stop(&srv);

	syslog(LOG_NOTICE, "graceful termination\n");
	return (EXIT_SUCCESS);
}
