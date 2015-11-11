/*
 * wmrq.c:
 * Client component to query for real-time data
 *
 * This software may be freely used and distributed according to the terms
 * of the GNU GPL version 2 or 3. See LICENSE for more information.
 *
 * Copyright (c) 2015 David Čepelík <cepelik@gymlit.cz>
 */

#include <stdio.h>
#include <string.h>
#include <err.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>

#include "wmrdata.h"
#include "loggers/file.h"

#define ARRAY_ELEM		unsigned char
#define ARRAY_PREFIX(x)		byte_##x
#include "array.h"


#define BUF_LEN		256	/* B */
#define DEFAULT_PORT	20892


int
main(int argc, const char *argv[]) {
	int fd, ret, n;
	const char *hostname, *portstr;
	struct addrinfo hints;
	struct addrinfo *srvinfo, *rp;

	if (argc < 3)
		errx(1, "usage: %s <hostname> <portstr>\n", argv[0]);

	hostname = argv[1];
	portstr = argv[2];

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = 0;
	hints.ai_protocol = 0;

	if ((ret = getaddrinfo(hostname, portstr, &hints, &srvinfo)) != 0)
		errx( 1, "getaddrinfo failed for '%s': %s\n", hostname, gai_strerror(ret));

	for (rp = srvinfo; rp != NULL; rp = rp->ai_next) {
		fd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);

		if (fd == -1)
			continue;

		if (connect(fd, rp->ai_addr, rp->ai_addrlen) != -1)
			break;

		close(fd);
	}

	freeaddrinfo(srvinfo);

	if (rp == NULL) { /* no address succeeded */
		errx(1, "Cannot connect to '%s'\n", hostname);
	}

	fprintf(stderr, "Connected to '%s'\n", hostname);

	struct byte_array arr;
	byte_array_init(&arr);

	unsigned char buf[BUF_LEN];
	while ((n = read(fd, &buf, sizeof(buf))) > 0) {
		fprintf(stderr, "Will push %u bytes\n", n);
		byte_array_push_n(&arr, &buf[0], n);
	}

	fprintf(stderr, "Buffer contents: ");
	for (uint i = 0; i < arr.size; i++) {
		fprintf(stderr, "%03u ", arr.elems[i]);
	}
	fprintf(stderr, "\n");

	wmr_wind deserialize_wind(struct byte_array *arr);

	wmr_reading reading = {
		.type = WMR_WIND,
		.wind = deserialize_wind(&arr)
	};

	log_to_file(&reading, stdout);

	(void)close(fd);
	fprintf(stderr, "Received %zu bytes\n", arr.size);
	return (0);
}
