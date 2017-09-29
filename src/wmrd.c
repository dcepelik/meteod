/*
 * Weather logging daemon
 *
 * This free software is distributed under the terms
 * of the MIT license. See LICENSE for more information.
 *
 * Copyright (c) 2015-2017 David Čepelík <d@dcepelik.cz>
 */

#include "daemon.h"
#include "log.h"
#include "rrd-logger.h"
#include "server.h"
#include "wmr200.h"

#include <assert.h>
#include <err.h>
#include <getopt.h>
#include <libgen.h>
#include <pthread.h>
#include <semaphore.h>
#include <signal.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#define RECONNECT_INTERVAL_MAX	120 /* seconds */

/* TODO make these configurable */
bool reconnect_on_error = true;
unsigned reconnect_interval_default = 3; /* seconds */

char *argv0;
unsigned reconnect_interval;
sem_t ev_sem;

volatile sig_atomic_t ev_error;	/* an error occured */
volatile sig_atomic_t ev_alarm;	/* alarm has expired */
volatile sig_atomic_t ev_quit;	/* quit request */

/*
 * Handle SIGINT, SIGINT and SIGALRM.
 */
static void signal_dispatch(int signum)
{
	switch (signum) {
	case SIGINT:
	case SIGTERM:
		ev_quit = true;
		break;
	case SIGALRM:
		ev_alarm = true;
		break;
	default:
		return;
	}

	sem_post(&ev_sem);
}

/*
 * Error handler. Called when a fatal error occurs while talking to
 * the station.
 */
static void error_handler(struct wmr200 *wmr, void *arg)
{
	(void) arg;
	(void) wmr;
	fprintf(stderr, "error_handler\n");
	if (!ev_error)
		sem_post(&ev_sem);
	ev_error = true;
}

static void usage(char *argv0)
{
	(void) argv0;
}

/*
 * Schedule a reconnection attempt and increase the reconnection interval.
 *
 * NOTE: Upon successful connection, the reconnection interval will be reset
 *       to default value. It will never exceed RECONNECT_INTERVAL_MAX.
 */
void schedule_reconnect(void)
{
	alarm(reconnect_interval);
	log_info("Will attempt to reconnect in %lu seconds.", reconnect_interval);
	reconnect_interval = MIN(2 * reconnect_interval, RECONNECT_INTERVAL_MAX);
}

/*
 * Daemon entry point. This does several things.
 *
 * First of all, this is a simple event loop. It uses the semaphore ev_sem,
 * on which it waits until an event of interest occurs. The events are:
 *
 *     - A SIGINT/SIGTERM signal is received. In that case, the daemon should
 *       shut down gracefully, no matter what.
 *
 *     - error_handler is invoked by the wmr200 module (running in a different
 *       thread), which means a fatal error has occurred, such as the station was
 *       unplugged from the machine, an invalid packet was received, etc. In that
 *       case, we want to terminate the communication. (And reconnect later
 *       after some period of waiting.)
 *
 *     - A SIGALRM signal is received. In that case, we want to start connecting
 *       again, because the reconnection delay has expired.
 */
int main(int argc, char *argv[])
{
	(void) argc;
	(void) argv;

	struct wmr200 *wmr;
	struct sigaction sa;
	struct rrd_logger rrd;
	sigset_t set;
	sigset_t oldset;
	bool running = false;
	struct wmr_server srv;

	argv0 = basename(argv[0]);

	memset(&sa, 0, sizeof(sa));
	sa.sa_handler = signal_dispatch;
	sigaction(SIGTERM, &sa, NULL);
	sigaction(SIGINT, &sa, NULL);
	sigaction(SIGALRM, &sa, NULL);

	log_open_syslog();
	sem_init(&ev_sem, false, 0);

	wmr_init();

	reconnect_interval = reconnect_interval_default;

connect:
	/*
	 * Block SIGINT, SIGTERM and SIGALRM. Spawned threads will inherit
	 * the sigmask, so signals will be received by this "main" thread.
	 */
	sigemptyset(&set);
	sigaddset(&set, SIGINT);
	sigaddset(&set, SIGTERM);
	sigaddset(&set, SIGALRM);
	pthread_sigmask(SIG_BLOCK, &set, &oldset);

	assert(!running);

	if ((wmr = wmr_open()) != NULL) {
		wmr_set_error_handler(wmr, error_handler, NULL);
		if (wmr_start(wmr) == 0) {
			running = true;
			reconnect_interval = reconnect_interval_default;

			rrd_logger_init(&rrd);
			rrd.cfg.rrd_root = "/tmp";
			rrd.cfg.wind_rrd = "wind.rrd";
			rrd.cfg.rain_rrd = "rain.rrd";
			rrd.cfg.uvi_rrd = "uvi.rrd";
			rrd.cfg.baro_rrd = "baro.rrd";
			rrd.cfg.temp_N_rrd = "temp%u.rrd";

			wmr_register_logger(wmr, rrd_log_reading, &rrd);

			server_init(&srv, wmr);
			server_start(&srv);
		}
		else {
			wmr_close(wmr);
		}
	}

	if (!running) {
		if (reconnect_on_error)
			schedule_reconnect();
		else
			goto quit;
	}

	pthread_sigmask(SIG_SETMASK, &oldset, NULL);

	/*
	 * Wait here until either an alarm expires (which implies a reconnection
	 * attempt should proceed), or a SIGINT/SIGTERM arrives, or an error occurs.
	 *
	 * NOTE: sem_wait may be interrupted by a signal, hence the loop.
	 */
wait:
	while (sem_wait(&ev_sem) != 0);

	if (ev_alarm) {
		ev_alarm = false;
		goto connect;
	}

	/*
	 * We're handling a quit request or an error has occurred. In any case,
	 * if we're connected, we should disconnect now.
	 */
	if (running) {
		wmr_stop(wmr);
		wmr_close(wmr);
		server_stop(&srv);
		rrd_logger_free(&rrd);
		running = false;
	}

	/*
	 * OK, so an error occurred. If no signal to quit was received, schedule
	 * a reconnection attempt.
	 */
	if (!ev_quit && ev_error && reconnect_on_error) {
		ev_error = false;
		schedule_reconnect();
		goto wait;
	}

	if (ev_quit)
		log_info("Shutting down gracefully on SIGINT/SIGTERM");

quit:
	wmr_end();
	return ev_error ? EXIT_FAILURE : EXIT_SUCCESS;
}
