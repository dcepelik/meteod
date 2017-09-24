/*
 * loggers/server.c:
 * Make readings available to the server
 *
 * This software may be freely used and distributed according to the terms
 * of the GNU GPL version 2 or 3. See LICENSE for more information.
 *
 * Copyright (c) 2015 David Čepelík <cepelik@gymlit.cz>
 */

#include "../server.h" /* TODO */


/*
 * public interface
 */


void
server_push_reading(struct wmr200 *wmr, wmr_reading *reading, void *arg)
{
	(void) wmr;
	(void) reading;
	(void) arg;

	/* TODO implement this */
}
