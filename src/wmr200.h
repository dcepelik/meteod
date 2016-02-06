/*
 * wmr200.h:
 * Oregon Scientific WMR200 USB HID communication wrapper
 *
 * This software may be freely used and distributed according to the terms
 * of the GNU GPL version 2 or 3. See LICENSE for more information.
 *
 * Copyright (c) 2015 David Čepelík <cepelik@gymlit.cz>
 */

#ifndef WMR200_H
#define	WMR200_H

#include "wmrdata.h"
#include "common.h"

#include <stdio.h>
#include <hidapi.h>
#include <pthread.h>


#define	WMR200_FRAME_SIZE		8
#define	WMR200_MAX_TEMP_SENSORS		10


typedef void (*wmr_handler_t)(wmr_reading *reading, void *arg);


struct wmr_handler;

typedef struct { /* move to .c */
	hid_device *dev;		/* HIDAPI device handle */
	pthread_t mainloop_thread;	/* main loop thread id */
	pthread_t heartbeat_thread;	/* heartbeat loop thread id */
	struct wmr_handler *handler;	/* handlers of "reading ready" event */
	wmr_latest_data latest;		/* latest readings of all kinds */
	wmr_meta meta;			/* system meta-packet being made */
	time_t conn_since;		/* time the connection was started */

	/* receive buffer */
	uchar buf[WMR200_FRAME_SIZE];
	uint_t buf_avail;
	uint_t buf_pos;

	/* packet being processed */
	uchar *packet;
	uchar packet_type;
	uint_t packet_len;
} wmr200;


void wmr_init();


void wmr_end();


wmr200 *wmr_open();


int wmr_start(wmr200 *wmr);


void wmr_stop(wmr200 *wmr);


void wmr_close(wmr200 *wmr);


void wmr_add_handler(wmr200 *wmr, wmr_handler_t handler, void *arg);


#endif
