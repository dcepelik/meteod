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
#define WMR200_H

#include "wmrdata.h"
#include "common.h"

#include <stdio.h>
#include <hidapi.h>


#define WMR200_FRAME_SIZE		8
#define WMR200_MAX_TEMP_SENSORS		10


struct wmr_handler;

typedef struct {
	hid_device *dev;		// HIDAPI device handle

	uchar buf[WMR200_FRAME_SIZE];	// receive buffer
	uint buf_avail;
	uint buf_pos;

	uchar *packet;			// packet being processed
	uchar packet_type;
	uint packet_len;

	struct wmr_handler *handler;	// handlers
} wmr200;


void wmr_init();


void wmr_end();


wmr200 *wmr_open();


void wmr_close(wmr200 *wmr);


void wmr_main_loop(wmr200 *wmr);


void wmr_set_handler(wmr200 *wmr, void (*handler)(wmr_reading *));


#endif
