/*
 * loggers/server.c:
 * Server component of the WMR daemon
 *
 * This software may be freely used and distributed according to the terms
 * of the GNU GPL version 2 or 3. See LICENSE for more information.
 *
 * Copyright (c) 2015 David Čepelík <cepelik@gymlit.cz>
 */


#include "server.h"
#include "strbuf.h"
#include "serialize.h"

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <err.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <rpc/xdr.h>
#include "time.h"


#define ARRAY_ELEM		unsigned char
#define ARRAY_PREFIX(x)		byte_##x
#include "array.h"


void
log_to_server(wmr_server *srv, wmr_reading *reading) {
	switch (reading->type) {
	case WMR_WIND:
		srv->data.wind = reading->wind;
		break;

	case WMR_RAIN:
		srv->data.rain = reading->rain;
		break;

	case WMR_UVI:
		srv->data.uvi = reading->uvi;
		break;

	case WMR_BARO:
		srv->data.baro = reading->baro;
		break;

	case WMR_TEMP:
		srv->data.temp[reading->temp.sensor_id] = reading->temp;
		break;

	case WMR_STATUS:
		srv->data.status = reading->status;
		break;
	}
}


void
send_latest_data(wmr_server *server) {
}


void
server_start(wmr_server *server) {
	int fd, newsock;

	struct sockaddr_in in = {
		.sin_family = AF_INET,
		.sin_port = htons(20892),
		.sin_addr.s_addr = htonl(INADDR_ANY),
	};

	if ((fd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
		err(1, "socket");
	}

	if (bind(fd, (struct sockaddr *) &in, sizeof(in)) == -1) {
		err(1, "bind");
	}

	if (listen(fd, SOMAXCONN) == -1) {
		err(1, "listen");
	}

	wmr_reading reading = {
		.type = WMR_TEMP,
		.temp = {
			.time = time(NULL),
			.sensor_id = 42,
			.humidity = 77,
			.heat_index = 2,
			.temp = 1.445,
			.dew_point = 17.222228,
		}
	};

	struct byte_array arr;
	byte_array_init(&arr);

	serialize_reading(&arr, &reading);

	fprintf(stderr, "Server array size: %zu\n", arr.size);


	for (;;) {
		if ((newsock = accept(fd, NULL, 0)) == -1) {
			err(1, "accept");
		}

		fprintf(stderr, "Accepted client\n");

		if (write(newsock, arr.elems, arr.size) != arr.size) {
			fprintf(stderr, "Cannot send %zu bytes over nework\n", arr.size);
		}

		break;
	}

	(void)close(newsock);
}
