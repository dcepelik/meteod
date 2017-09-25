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

#include <assert.h>
#include <err.h>
#include <getopt.h>
#include <libgen.h>
#include <pthread.h>
#include <signal.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

pthread_mutex_t mutex;
pthread_cond_t error_or_signal;
bool error;
unsigned reconnect_interval_default = 3; /* s */
bool reconnect_on_error = true;
char *argv0;

#define RECONNECT_INTERVAL_MAX	300

static void signal_dispatch(int signum)
{
	switch (signum) {
	case SIGINT:
	case SIGTERM:
		pthread_mutex_lock(&mutex);
		pthread_cond_signal(&error_or_signal);
		pthread_mutex_unlock(&mutex);
		break;
	}
}


/*
 * Error handler. Called when a fatal error occurs while talking to
 * the station.
 */
static void error_handler(struct wmr200 *wmr, void *arg)
{
	(void) arg;
	(void) wmr;

	pthread_mutex_lock(&mutex);
	error = 1;
	pthread_cond_signal(&error_or_signal);
	pthread_mutex_unlock(&mutex);
}


static void usage(char *argv0)
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
	(void) argc;
	(void) argv;

	sigset_t set;
	sigset_t oldset;
	struct wmr200 *wmr;
	struct sigaction sa;
	unsigned reconnect_interval = reconnect_interval_default;

	argv0 = basename(argv[0]);

	log_open_syslog();

	memset(&sa, 0, sizeof(sa));
	sa.sa_handler = signal_dispatch;
	sigaction(SIGTERM, &sa, NULL);
	sigaction(SIGINT, &sa, NULL);

	/*
	 * Block SIGINT/SIGTERM in spawned threads. This way we can be sure
	 * that the main thread will receive the signal.
	 */
	sigemptyset(&set);
	sigaddset(&set, SIGINT);
	sigaddset(&set, SIGTERM);
	pthread_sigmask(SIG_BLOCK, &set, &oldset);

	pthread_mutex_init(&mutex, NULL);
	pthread_cond_init(&error_or_signal, NULL);

	wmr_init();

connect:
	pthread_mutex_lock(&mutex);

	if (error) {
		log_info("Waiting %u seconds before reconnecting.", reconnect_interval);
		sleep(reconnect_interval);
		/* TODO what if signal arrives here? */
		reconnect_interval = MIN(2 * reconnect_interval, RECONNECT_INTERVAL_MAX);
	}

	if ((wmr = wmr_open()) == NULL) {
		log_error("wmr_open: connection failed\n");
		error = true;
		pthread_mutex_unlock(&mutex);
		goto quit;
	}

	wmr_set_error_handler(wmr, error_handler, NULL);

	if (wmr_start(wmr) != 0) {
		log_error("wmr_start: cannot start communication with the station");
		error = true;
		goto disconnect;
	}

	error = false;
	reconnect_interval = reconnect_interval_default;

	//daemonize(argv0);

	/*
	 * Wait here until either an error occurs, or SIGINT/SIGTERM is received.
	 */
	pthread_sigmask(SIG_SETMASK, &oldset, NULL);
	pthread_cond_wait(&error_or_signal, &mutex);
	pthread_mutex_unlock(&mutex);

	wmr_stop(wmr);

disconnect:
	wmr_close(wmr);

quit:
	if (error) {
		log_error("Connection was terminated because an error occured.");

		if (reconnect_on_error)
			goto connect;
	}
	else {
		log_info("Terminating on SIGINT/SIGTERM.");
	}

	wmr_end();

	pthread_mutex_destroy(&mutex);
	pthread_cond_destroy(&error_or_signal);

	return error ? EXIT_FAILURE : EXIT_SUCCESS;
}

