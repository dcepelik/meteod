/*
 * server.h
 *
 *
 * This software may be freely used and distributed according to the terms
 * of the GNU GPL version 2 or 3. See LICENSE for more information.
 *
 * Copyright (c) 2015 David Čepelík <cepelik@gymlit.cz>
 */

#ifndef SERVER_H
#define SERVER_H


struct latest_data;

typedef struct {
	
} wmr_server;


void
server_start(wmr_server *srv);


void
server_stop(wmr_server *srv);


#endif
