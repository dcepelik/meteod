/*
 * log.h:
 * Logging and debugging functions
 *
 * This software may be freely used and distributed according to the terms
 * of the GNU GPL version 2 or 3. See LICENSE for more information.
 *
 * Copyright (c) 2015 David Čepelík <cepelik@gymlit.cz>
 */

#ifndef LOG_H
#define LOG_H

#include <syslog.h>


void log_open_syslog(void);

void log_msg(int priority, char *msg, ...);

void log_warning(char *msg, ...);

void log_error(char *msg, ...);

void log_debug(char *msg, ...);

void log_info(char *msg, ...);

void log_exit(char *msg, ...);

#endif
