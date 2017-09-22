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
#include "log.h"
#include "string.h"

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <err.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <string.h>


static void
detach_from_parent(void)
{
	pid_t pid1, pid2;
	pid_t sid;
	
	pid1 = fork();
	if (pid1 < 0)
		log_exit("Cannot fork\n");
	else if (pid1 > 0)
		exit(EXIT_SUCCESS);

	fclose(stdin);
	fclose(stdout);
	fclose(stderr);

	sid = setsid();
	if (sid < 0)
		log_exit("Could not create new session\n");

	pid2 = fork();
	if (pid2 < 0)
		log_exit("Cannot fork second time\n");
	else if (pid2 > 0)
		exit(EXIT_SUCCESS);

	if (setpgrp() == -1)
		log_exit("Cannot set process group\n");

	log_info("Running as PID %d\n", pid2);
}


static void
jail(void)
{
	int ret;
	char *chdir_dir = "/var/wmrd";

	umask(UMASK);

	if ((ret = chdir(chdir_dir)) == -1)
		log_exit("Cannot chdir to %s (%s), will exit\n",
			chdir_dir, strerror(errno));

	if ((ret = chroot(chdir_dir)) == -1)
		log_exit("Cannot chroot to %s (%s), will exit\n",
			chdir_dir, strerror(errno));
}


static void
drop_root_privileges(void)
{
	gid_t group_id = 1006;
	uid_t user_id = 1006;

	if (setgid(group_id) == -1)
		log_exit("Canot setgid to %d (%s), will exit\n",
			group_id, strerror(errno));

	if (setuid(user_id) == -1)
		log_exit("Cannot setuid to %d (errno %d), will exit\n",
			errno, user_id);

	if (setuid(0) != -1)
		log_exit("Root privileges not relinquished correctly");

	log_info("Dropped root priviliges\n");
}


/*
 * public interface
 */


void
daemonize(const char *argv0)
{
	(void) argv0;

	detach_from_parent();
	jail();
	drop_root_privileges();
}
