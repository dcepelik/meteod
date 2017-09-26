/*
 * wmrq.c:
 * Client component to query for real-time data
 *
 * This software may be freely used and distributed according to the terms
 * of the GNU GPL version 2 or 3. See LICENSE for more information.
 *
 * Copyright (c) 2015 David Čepelík <cepelik@gymlit.cz>
 */

#include "logger-yaml.h"
#include "serialize.h"
#include "wmr200.h"
#include "wmrdata.h"

#include <arpa/inet.h>
#include <err.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

#define	ARRAY_ELEM		unsigned char
#define	ARRAY_PREFIX(x)		byte_##x
#include "array.h"


#define	BUF_LEN		256
#define	DEFAULT_PORT	20892


int
main(int argc, const char *argv[])
{
	uint i;
	int fd, ret, n;
	const char *hostname, *portstr;
	struct addrinfo hints;
	struct addrinfo *head, *cur;

	if (argc < 3)
		errx(EXIT_FAILURE, "usage: %s <hostname> <portstr>\n", argv[0]);

	hostname = argv[1];
	portstr = argv[2];

	memset(&hints, 0, sizeof (hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = 0;
	hints.ai_protocol = 0;

	if ((ret = getaddrinfo(hostname, portstr, &hints, &head)) != 0)
		errx(EXIT_FAILURE, "getaddrinfo failed for '%s': %s\n",
			hostname, gai_strerror(ret));

	for (cur = head; cur != NULL; cur = cur->ai_next) {
		fd = socket(cur->ai_family, cur->ai_socktype, cur->ai_protocol);

		if (fd == -1)
			continue;

		if (connect(fd, cur->ai_addr, cur->ai_addrlen) == 0)
			break;

		(void) close(fd);
	}

	freeaddrinfo(head);


	/* when cur == NULL, we are not connected  */
	if (cur == NULL)
		errx(EXIT_FAILURE, "Cannot connect to '%s'\n", hostname);

	DEBUG_MSG("Connected to '%s'", hostname);

	struct byte_array arr;
	byte_array_init(&arr);

	unsigned char buf[BUF_LEN];
	while ((n = read(fd, &buf, sizeof (buf))) > 0) {
		byte_array_push_n(&arr, &buf[0], n);
	}

	DEBUG_MSG("Read %zu bytes from network", arr.size);

	struct wmr_latest_data data;
	deserialize_data(&arr, &data);

	yaml_push_reading(NULL, &data.wind, stdout);
	yaml_push_reading(NULL, &data.rain, stdout);
	yaml_push_reading(NULL, &data.uvi, stdout);
	
	for (i = 0; i < WMR200_MAX_TEMP_SENSORS; i++)
		yaml_push_reading(NULL, &data.temp[i], stdout);

	yaml_push_reading(NULL, &data.baro, stdout);
	yaml_push_reading(NULL, &data.status, stdout);
	yaml_push_reading(NULL, &data.meta, stdout);

	(void) close(fd);
	DEBUG_MSG("%s", "Connection to server closed");

	return (EXIT_SUCCESS);
}
