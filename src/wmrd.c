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
#include <err.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <syslog.h>
#include <errno.h>

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


static void
daemonize(const char *argv0) {
	pid_t pid, newpid;
	pid_t sid;
	gid_t group_id = 1001;
	uid_t user_id = 1001;

	extern int errno;
	
	pid = getpid();
	fprintf(stderr, "Running as interactive process with PID %d\n", pid);

	fprintf(stderr, "Will fork, see you in the syslog\n");
	newpid = fork();

	if (newpid < 0)
		err(EXIT_FAILURE, "fork");
	else if (newpid > 0)
		exit(EXIT_SUCCESS);

	close(STDIN_FILENO);
	close(STDOUT_FILENO);
	close(STDERR_FILENO);

	syslog(LOG_INFO, "%s running as PID %d\n", argv0, newpid);

	umask(227);
	if (chdir("/var/wmrd") == -1) {
		syslog(LOG_ERR, "Cannot chdir to /, will exit\n");
		exit(EXIT_FAILURE);
	}

	if (chroot("/var/wmrd") == -1) {
		syslog(LOG_ERR, "Cannot chroot to /var/wmrd, will exit\n");
		exit(EXIT_FAILURE);
	}

	openlog(argv0, LOG_NOWAIT | LOG_PID, LOG_USER);

	newpid = getpid();

	sid = setsid();
	if (sid < 0) {
		syslog(LOG_ERR, "Could not create new process group\n");
		exit(EXIT_FAILURE);
	}

	syslog(LOG_INFO, "Running with SID %d\n", sid);

	if (setgid(group_id) == -1) {
		syslog(LOG_ERR, "Canot setgid to %d (%s), will exit\n",
			group_id, strerror(errno));
		exit(EXIT_FAILURE);
	}

	if (setuid(user_id) == -1) {
		syslog(LOG_ERR, "Cannot setuid to %d (errno %d), will exit\n",
			errno, user_id);
		exit(EXIT_FAILURE);
	}

	syslog(LOG_INFO, "Dropped root priviliges\n");
}


int
main(int argc, const char *argv[]) {
	daemonize(argv[0]);

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

	syslog(LOG_NOTICE, "%s: graceful termination\n", argv[0]);
	return (EXIT_SUCCESS);
}
