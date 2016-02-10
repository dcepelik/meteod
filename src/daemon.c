/*
 * daemon.c:
 * Run this process as a secure system daemon
 *
 * This software may be freely used and distributed according to the terms
 * of the GNU GPL version 2 or 3. See LICENSE for more information.
 *
 * Copyright (c) 2015 David Čepelík <cepelik@gymlit.cz>
 */

#include "daemon.h"
#include "common.h"

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <err.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <syslog.h>
#include <errno.h>
#include <string.h>


static void
detach_from_parent(void)
{
	pid_t pid1, pid2;
	pid_t sid;
	
	pid1 = fork();
	if (pid1 < 0)
		err(EXIT_FAILURE, "fork");
	else if (pid1 > 0)
		exit(EXIT_SUCCESS);

	fclose(stdin);
	fclose(stdout);
	fclose(stderr);

	sid = setsid();
	if (sid < 0) {
		syslog(LOG_ERR, "Could not create new session\n");
		exit(EXIT_FAILURE);
	}

	pid2 = fork();
	if (pid2 < 0) {
		syslog(LOG_ERR, "Cannot fork() second time\n");
		exit(EXIT_FAILURE);
	}
	else if (pid2 > 0) {
		exit(EXIT_SUCCESS);
	}

	if (setpgrp() == -1) {
		syslog(LOG_ERR, "Cannot set process group\n");
		exit(EXIT_FAILURE);
	}

	syslog(LOG_INFO, "Running as PID %d\n", pid2);
}


static void
jail(void)
{
	umask(UMASK);

	if (chdir("/var/wmrd") == -1) {
		syslog(LOG_ERR, "Cannot chdir to /, will exit\n");
		exit(EXIT_FAILURE);
	}

	if (chroot("/var/wmrd") == -1) {
		syslog(LOG_ERR, "Cannot chroot to /var/wmrd, will exit\n");
		exit(EXIT_FAILURE);
	}
}


static void
drop_root_privileges(void)
{
	gid_t group_id = 1006;
	uid_t user_id = 1006;

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

	if (setuid(0) != -1) {
		syslog(LOG_ERR, "Root privileges not relinquished correctly");
		exit(EXIT_FAILURE);
	}

	syslog(LOG_INFO, "Dropped root priviliges\n");
}


/*
 * public interface
 */


void
daemonize(const char *argv0)
{
	detach_from_parent();
	jail();
	drop_root_privileges();
}
