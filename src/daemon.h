/*
 * daemon.h:
 * Run this process as a secure system daemon
 *
 * This software may be freely used and distributed according to the terms
 * of the GNU GPL version 2 or 3. See LICENSE for more information.
 *
 * Copyright (c) 2015 David Čepelík <cepelik@gymlit.cz>
 */

#ifndef DAEMON_H
#define DAEMON_H

#define UMASK	227


void daemonize(const char *argv0);


#endif
