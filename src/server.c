/*
 * Make data available over TCP/IP.
 */

#include "log.h"
#include "server.h"

#include <assert.h>
#include <err.h>
#include <netdb.h>
#include <netinet/in.h>
#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

#define	DEFAULT_PORT		20892

static void print_wind(struct wmr_wind *wind, int fd)
{
	dprintf(fd, "wind\tdir=%s\tgust_speed=%.1f m/s\tavg_speed=%.1f m/s\tchill=%.1f \u00B0C\n",
		wind->dir,
		wind->gust_speed,
		wind->avg_speed,
		wind->chill);
}

static void print_rain(struct wmr_rain *rain, int fd)
{
	dprintf(fd, "rain\trate=%.1f mm/m^2\taccum_hour=%.1f mm/m^2\t"
		"accum_24h=%1.f mm/m^2\taccum_2007=%.1f mm/m^2\n",
		rain->rate,
		rain->accum_hour,
		rain->accum_24h,
		rain->accum_2007);
}

static void print_uvi(struct wmr_uvi *uvi, int fd)
{
	dprintf(fd, "uvi\tindex=%u\n", uvi->index);
}

static void print_baro(struct wmr_baro *baro, int fd)
{
	dprintf(fd, "baro\talt_pressure=%u hPa\tforecast=%s\n",
		baro->alt_pressure,
		baro->forecast);
}

static void print_temp(struct wmr_temp *temp, int fd)
{
	dprintf(fd, "temp\tsensor=%s\ttemp=%.1f \u00B0C\thumidity=%u %%\tdew_point=%.1f \u00B0C\n",
		"console",
		temp->temp,
		temp->humidity,
		temp->dew_point);
}

static void print_status(struct wmr_status *status, int fd)
{
	dprintf(fd, "status\twind_bat=%s\ttemp_bat=%s\train_bat=%s\tuv_bat=%s\t"
		"wind_sensor=%s\ttemp_sensor=%s\train_sensor=%s\tuv_sensor=%s\t"
		"rtc_signal=%s\n",
		status->wind_bat, status->temp_bat, status->rain_bat, status->uv_bat,
		status->wind_sensor, status->temp_sensor, status->rain_sensor,
		status->uv_sensor, status->rtc_signal_level);
}

static void print_meta(struct wmr_meta *meta, int fd)
{

	(void) meta;
	dprintf(fd, "meta\tnpackets=%u\tnfailed=%u\tnframes=%u\terror_rate=%.1f\t"
		"nbytes=%lu\tlatest_packet=%s\tuptime=%02lu:%02lu\n",
		meta->num_packets,
		meta->num_failed,
		meta->num_frames,
		meta->error_rate,
		meta->num_bytes,
		ctime(&meta->latest_packet),
		meta->uptime / 60, meta->uptime % 60);
}

static void print_reading(struct wmr_reading *reading, int fd)
{
	switch (reading->type) {
	case 0: /* not measured yet */
		break;
	case WMR_WIND:
		print_wind(&reading->wind, fd);
		break;
	case WMR_RAIN:
		print_rain(&reading->rain, fd);
		break;
	case WMR_UVI:
		print_uvi(&reading->uvi, fd);
		break;
	case WMR_BARO:
		print_baro(&reading->baro, fd);
		break;
	case WMR_TEMP:
		print_temp(&reading->temp, fd);
		break;
	case WMR_STATUS:
		print_status(&reading->status, fd);
		break;
	case WMR_META:
		print_meta(&reading->meta, fd);
		break;
	default:
		assert(0);
	}
}

static void mainloop(struct wmr_server *srv)
{
	struct wmr_latest_data latest;
	int fd;
	size_t i;

	log_info("%s", "Entering server main loop");
	while (1) {
		/* POSIX.1: accept is a cancellation point */
		if ((fd = accept(srv->fd, NULL, 0)) == -1)
			err(1, "accept"); /* TODO don't use err */

		log_debug("Client accepted, fd = %u", fd);
		wmr_get_latest_data(srv->wmr, &latest);

		print_reading(&latest.wind, fd);
		print_reading(&latest.rain, fd);
		print_reading(&latest.baro, fd);
		print_reading(&latest.uvi, fd);
		for (i = 0; i < WMR200_MAX_TEMP_SENSORS; i++)
			print_reading(&latest.temp[i], fd);
		print_reading(&latest.meta, fd);
		print_reading(&latest.status, fd);

		close(fd);
		log_debug("%s", "Client socket closed");
	}
}

static void cleanup(void *arg)
{
	struct wmr_server *srv = (struct wmr_server *)arg;
	assert(srv->fd >= 0);
	close(srv->fd);
}

static void *mainloop_pthread(void *arg)
{
	struct wmr_server *srv = (struct wmr_server *)arg;

	pthread_cleanup_push(cleanup, srv);
	mainloop(srv);
	pthread_cleanup_pop(1);

	return NULL;
}

void server_init(struct wmr_server *srv, struct wmr200 *wmr)
{
	srv->wmr = wmr;
	srv->fd = srv->thread_id = -1;
}

int server_start(struct wmr_server *srv)
{
	struct addrinfo *ai_head, *ai_cur;
	struct addrinfo ai_hints; 
	int port = DEFAULT_PORT;
	char portstr[6];
	int optval = 1;
	int ret;

	memset(&ai_hints, 0, sizeof(ai_hints));
	ai_hints.ai_family = AF_UNSPEC;
	ai_hints.ai_socktype = SOCK_STREAM;
	ai_hints.ai_flags = AI_PASSIVE;

	snprintf(portstr, sizeof(portstr), "%u", port);

	if ((ret = getaddrinfo(NULL, portstr, &ai_hints, &ai_head)) != 0) {
		log_error("getaddrinfo: %s\n", gai_strerror(ret));
		return -1;
	}

	for (ai_cur = ai_head; ai_cur != NULL; ai_cur = ai_cur->ai_next) {
		srv->fd = socket(ai_cur->ai_family, ai_cur->ai_socktype,
			ai_cur->ai_protocol);

		if (srv->fd == -1)
			continue;

		setsockopt(srv->fd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));
		if (bind(srv->fd, ai_cur->ai_addr, ai_cur->ai_addrlen) == 0)
			break;

		close(srv->fd);
	}

	freeaddrinfo(ai_head);

	/* if ai_cur == NULL, we are not bound to any address  */
	if (ai_cur == NULL) {
		log_error("%s", "Cannot bind to any address");
		return -1;
	}

	if (listen(srv->fd, SOMAXCONN) == -1) {
		log_error("listen: %s", "Cannot start listening");
		return -1;
	}

	log_info("Server start successful, descriptor is %d", srv->fd);

	if (pthread_create(&srv->thread_id, NULL, mainloop_pthread, srv) != 0) {
		log_error("%s", "Cannot start server main loop thread");
		return -1;
	}

	return 0;
}

void server_stop(struct wmr_server *srv)
{
	pthread_cancel(srv->thread_id);
	pthread_join(srv->thread_id, NULL);
}
