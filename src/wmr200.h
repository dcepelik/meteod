#ifndef WMR200_H
#define WMR200_H

#include "wmrdata.h"
#include "common.h"

#include <stdio.h>
#include <hidapi.h>


#define WMR200_FRAME_SIZE		8


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
