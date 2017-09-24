/*
 * wmrd.c:
 * WMR logging daemon
 *
 * This software may be freely used and distributed according to the terms
 * of the GNU GPL version 2 or 3. See LICENSE for more information.
 *
 * Copyright (c) 2015 David Čepelík <cepelik@gymlit.cz>
 */

#include "daemon.h"
#include "log.h"
#include "logger-rrd.h"
#include "logger-server.h"
#include "logger-yaml.h"
#include "server.h"
#include "wmr200.h"

#include <err.h>
#include <getopt.h>
#include <pthread.h>
#include <signal.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>


volatile sig_atomic_t exit_flag = 0;
int flag_start_server = 0;

int port = 20892;

static struct option longopts[] = {
	{ "run-server",		no_argument,		NULL,		'S' },
	{ "port",		required_argument,	&port,		'p' },
	{ "to-file",		required_argument,	NULL,		'F' },
	{ "to-rrd",		required_argument,	NULL,		'R' },
};

static const char *optstr = "Sp:F:R:";


static void
signal_handler(int signum)
{
	switch (signum) {
	case SIGINT:
	case SIGTERM:
		exit_flag = 1;
	}
}


static void
usage(char *argv0)
{
	fprintf(stderr, "%s: usage: %s <options>\n", argv0, argv0);
	fprintf(stderr,
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
	FILE *fp;
	sigset_t set, oldset;
	struct wmr200 *wmr;
	wmr_server srv;
	int c;

	log_open_syslog();

	/* hide signals away from created threads */
	sigemptyset(&set);
	sigaddset(&set, SIGINT);
	sigaddset(&set, SIGTERM);
	pthread_sigmask(SIG_BLOCK, &set, &oldset);
	wmr_init();
	wmr = wmr_open();

	if (wmr == NULL)
		log_exit("wmr_open: no WMR200 handle returned\n");

	while ((c = getopt_long(argc, argv, optstr, longopts, NULL)) != -1) {
		switch (c) {

		case 'S':
			flag_start_server = 1;
			break;
			
		case 'F':
			if (strcmp(optarg, "-") == 0)
				fp = stdout;
			else if ((fp = fopen(optarg, "w")) == NULL)
				log_exit("Cannot open output file");

			wmr_register_logger(wmr, yaml_push_reading, fp);
			break;

		case 'R':
			wmr_register_logger(wmr, rrd_push_reading, optarg);
			break;

		default:
			usage(argv0);
			exit(EXIT_FAILURE);
		}
	}

	if (flag_start_server) {
		server_init(&srv, wmr);

		if (server_start(&srv) != 0)
			log_exit("Cannot start server\n");

		wmr_register_logger(wmr, server_push_reading, &srv);
	}

	if (wmr_start(wmr) != 0)
		log_msg(LOG_CRIT, "Cannot start communication with WMR200\n");

	pthread_sigmask(SIG_SETMASK, &oldset, NULL);

	struct sigaction sa;
	memset(&sa, 0, sizeof (sa));
	sa.sa_handler = signal_handler;
	sigaction(SIGTERM, &sa, NULL);
	sigaction(SIGINT, &sa, NULL);

	//daemonize(argv0);

	while (1) {
		pause(); /* wait here for a signal to arrive */
		if (exit_flag)
			break;
	}

	wmr_stop(wmr);
	wmr_close(wmr);
	wmr_end();

	server_stop(&srv);

	if (fp != NULL)
		fclose(fp);

	log_msg(LOG_NOTICE, "Graceful termination\n");
	return (EXIT_SUCCESS);
}

