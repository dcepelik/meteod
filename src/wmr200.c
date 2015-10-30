/**
 *  wmr200.c:
 *  Oregon Scientific WMR200 communication wrapper
 *
 *  Copyright (c) 2015 David Čepelík <cepelik@gymlit.cz>
 *
 *  This program may be licensed under GNU GPL version 2 or 3.
 *  See the LICENSE file for more information.
 */


#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <time.h>
#include <sys/time.h>

#include "common.h"
#include "wmr200.h"
#include "macros.h"


const char *LEVEL[] = { "ok", "low" };
const char *STATUS[] = { "ok", "failed" };


const char *FORECAST[] = {
	"partly_cloudy-day", "rainy", "cloudy",
	"sunny", "clear", "snowy",
	"partly_cloudy-night"
};


const char *WIND_DIRECTION[] = {
	"N", "NNE", "NE", "ENE",
	"E", "ESE", "SE", "SSE",
	"S", "SSW", "SW", "WSW",
	"W", "WNW", "NW", "NNW"
};


const char *SENSOR_NAMES[1 + MAX_EXT_SENSORS] = {
	"indoor",
	"1", "2", "3", "4", "5",
	"6", "7", "8", "9", "10"
};


struct wmr200 *wmr_global; // TODO
uint exit_flag = 0;


/******************** sending and receiving data ********************/


static uchar
read_byte(struct wmr200 *wmr) {
	if (wmr->buf_avail == 0) {
		int ret = hid_read(wmr->dev, wmr->buf, FRAME_SIZE);

		if (ret != FRAME_SIZE) {
			fprintf(stderr, "hid_read: cannot read frame, return was %i\n", ret);
			exit(1);
		}

		wmr->buf_avail = wmr->buf[0];
		wmr->buf_pos = 1;
	}

	wmr->buf_avail--;
	return wmr->buf[wmr->buf_pos++];
}


static void
send_cmd_frame(struct wmr200 *wmr, uchar cmd) {
	uchar data[FRAME_SIZE] = { 0x01, cmd };
	int ret = hid_write(wmr->dev, data, FRAME_SIZE);

	if (ret != FRAME_SIZE) {
		fprintf(stderr, "hid_write: cannot send %02x command frame, return was %i", cmd, ret);
		exit(1);
	}
}


static void
send_heartbeat(struct wmr200 *wmr) {
	DEBUG_MSG("Sending heartbeat to WMR200\n");
	send_cmd_frame(wmr, HEARTBEAT);
}


/******************** data processing ********************/


static void
invoke_handlers(struct wmr200 *wmr, struct wmr_reading *reading) {
	struct wmr_handler *handler = wmr->handler;
	while (handler != NULL) {
		handler->handler(reading);
		handler = handler->next;
	}
}


static void
process_wind_data(struct wmr200 *wmr, uchar *data) {
	uint dir_flag		= LOW(data[7]);
	float gust_speed	= (256 * LOW(data[10]) +      data[9])   / 10.0;
	float avg_speed		= (256 * LOW(data[11]) + HIGH(data[10])) / 10.0;
	float chill		= (data[12] - 32) / 1.8;

	struct wmr_reading reading = {
		.type = WIND_DATA,
		.wind = {
			.dir = WIND_DIRECTION[dir_flag],
			.gust_speed = gust_speed,
			.avg_speed = avg_speed,
			.chill = chill
		}
	};

	invoke_handlers(wmr, &reading);
}


static void
process_rain_data(struct wmr200 *wmr, uchar *data) {
	float rain_rate		= (256 * data[8]  +  data[7]) * 25.4;
	float rain_hour		= (256 * data[10] +  data[9]) * 25.4;
	float rain_24h		= (256 * data[12] + data[11]) * 25.4;
	float rain_accum 	= (256 * data[14] + data[13]) * 25.4;

	printf("\train.rate: %.2f\n", rain_rate);
	printf("\train.hour: %.2f\n", rain_hour);
	printf("\train.24h: %.2f\n", rain_24h);
	printf("\train.accum: %.2f\n", rain_accum);
}


static void
process_uvi_data(struct wmr200 *wmr, uchar *data) {
	uint index = LOW(data[7]);
	printf("\tuvi.index: %u\n", index);
}


static void
process_baro_data(struct wmr200 *wmr, uchar *data) {
	uint pressure		= 256 * LOW(data[8])  + data[7];
	uint alt_pressure	= 256 * LOW(data[10]) + data[9];
	uint forecast_flag	= HIGH(data[8]);

	// pressure, altitude pressure and forecast flag
	printf("\tbaro.pressure: %u\n", pressure);
	printf("\tbaro.alt_pressure: %u\n", alt_pressure);
	printf("\tbaro.forecast: %s\n", FORECAST[forecast_flag]);
}


static void
process_temp_humid_data(struct wmr200 *wmr, uchar *data) {
	int sensor_id = data[7] & 0xF;

	// TODO
	if (sensor_id > 1) {
		fprintf(stderr, "Unknown sensor, ID: %i\n", sensor_id);
		exit(1);
	}

	const char *sensor_name = SENSOR_NAMES[sensor_id];


	uint humidity   = data[10];
	uint heat_index = data[13];

	float temp = (256 * LOW(data[9]) + data[8]) / 10.0;
	if (HIGH(data[9]) == SIGN_NEGATIVE) temp = -temp;

	float dew_point = (256 * LOW(data[12]) + data[11]) / 10.0;
	if (HIGH(data[12]) == SIGN_NEGATIVE) dew_point = -dew_point;

	printf("\ttemp_hum.%s.humidity: %u\n", sensor_name, humidity);
	printf("\ttemp_hum.%s.heat_index: %u\n", sensor_name, heat_index);
	printf("\ttemp_hum.%s.temp: %.1f\n", sensor_name, temp);
	printf("\ttemp_hum.%s.dew_point: %.1f\n", sensor_name, dew_point);
}


static void
process_status_data(struct wmr200 *wmr, uchar *data) {
	uint wind_bat_flag		= NTH_BIT(0, data[4]);
	uint temp_hum_bat_flag		= NTH_BIT(1, data[4]);
	uint rain_bat_flag		= NTH_BIT(4, data[5]);
	uint uv_bat_flag		= NTH_BIT(5, data[5]);

	uint wind_sensor_flag 		= NTH_BIT(0, data[2]);
	uint temp_hum_sensor_flag	= NTH_BIT(1, data[2]);
	uint rain_sensor_flag		= NTH_BIT(4, data[3]);
	uint uv_sensor_flag		= NTH_BIT(5, data[3]);

	uint rtc_signal_flag		= NTH_BIT(8, data[4]);

	printf("\tstatus.wind.bat: %s\n", LEVEL[wind_bat_flag]);
	printf("\tstatus.temp_hum.bat: %s\n", LEVEL[temp_hum_bat_flag]);
	printf("\tstatus.rain.bat: %s\n", LEVEL[rain_bat_flag]);
	printf("\tstatus.uv.bat: %s\n", LEVEL[uv_bat_flag]);

	printf("\tstatus.wind.sensor: %s\n", STATUS[wind_sensor_flag]);
	printf("\tstatus.temp_hum.sensor: %s\n", STATUS[temp_hum_sensor_flag]);
	printf("\tstatus.rain.sensor: %s\n", STATUS[rain_sensor_flag]);
	printf("\tstatus.uv.sensor: %s\n", STATUS[uv_sensor_flag]);

	printf("\tstatus.rtc.signal: %s\n", LEVEL[rtc_signal_flag]);
}


static void
process_historic_data(struct wmr200 *wmr, uchar *data) {
	process_rain_data(wmr, data);
	process_wind_data(wmr, data + 13);
	process_uvi_data(wmr, data + 20);
	process_baro_data(wmr, data + 21);
	process_temp_humid_data(wmr, data + 26);

	uint ext_sensor_count = data[32];
	if (ext_sensor_count > MAX_EXT_SENSORS) {
		fprintf(stderr, "Too many external sensors\n");
		exit(1);
	}

	for (uint i = 0; i < ext_sensor_count; i++) {
		process_temp_humid_data(wmr, data + 33 + (7 * i));
	}
}


/******************** packet processing ********************/


static time_t
parse_packet_time(struct wmr200 *wmr) {
	struct tm tm = {
		.tm_year	= (2000 + wmr->packet[6]) - 1900,
		.tm_mon		= wmr->packet[5],
		.tm_mday	= wmr->packet[4],
		.tm_hour	= wmr->packet[3],
		.tm_min		= wmr->packet[2],
		.tm_sec		= 0,
		.tm_isdst	= -1
	};

	return mktime(&tm);
}


static uint
calc_packet_checksum(struct wmr200 *wmr) {
	uint sum = 0;
	for (uint i = 0; i < wmr->packet_len - 2; i++) {
		sum += wmr->packet[i];
	}

	return sum;
}


static void
dispatch_packet(struct wmr200 *wmr) {
	switch (wmr->packet_type) {
	case HISTORIC_DATA:
		process_historic_data(wmr, wmr->packet);
		break;
	case WIND_DATA:
		process_wind_data(wmr, wmr->packet);
		break;
	case RAIN_DATA:
		process_rain_data(wmr, wmr->packet);
		break;
	case UVI_DATA:
		process_uvi_data(wmr, wmr->packet);
		break;
	case BARO_DATA:
		process_baro_data(wmr, wmr->packet);
		break;
	case TEMP_HUMID_DATA:
		process_temp_humid_data(wmr, wmr->packet);
		break;
	case STATUS_DATA:
		process_status_data(wmr, wmr->packet);
		break;
	default:
		DEBUG_MSG("Ignoring unknown packet %u\n", wmr->packet_type);
	}
}


static void
main_loop(struct wmr200 *wmr) {
	while (1) {
		wmr->packet_type = read_byte(wmr);

act_on_packet_type:
		switch (wmr->packet_type) {
		case HISTORIC_DATA_NOTIF:
			DEBUG_MSG("Data logger contains some unprocessed historic records\n");
			DEBUG_MSG("Issuing REQUEST_HISTORIC_DATA command\n");

			send_cmd_frame(wmr, REQUEST_HISTORIC_DATA);
			continue;

		case LOGGER_DATA_ERASE:
			DEBUG_MSG("Data logger database purge successful\n");
			continue;

		case COMMUNICATION_STOP:
			// ignore, sent as response to prev COMMUNICATION_STOP request
			DEBUG_MSG("Ignoring COMMUNICATION_STOP packet\n");
			break;
		}

		wmr->packet_len = read_byte(wmr);
		if (wmr->packet_len >= 0xD0 && wmr->packet_len <= 0xDF) {
			// this is a wmr->packet type mark, not wmr->packet length
			wmr->packet_type = wmr->packet_len;
			goto act_on_packet_type;
		}

		wmr->packet = malloc_safe(wmr->packet_len);
		wmr->packet[0] = wmr->packet_type;
		wmr->packet[1] = wmr->packet_len;

		for (uint i = 2; i < wmr->packet_len; i++) {
			wmr->packet[i] = read_byte(wmr);
		}

		uint recv_checksum
			= 256 * wmr->packet[wmr->packet_len - 1]
			+ wmr->packet[wmr->packet_len - 2];

		if (recv_checksum != calc_packet_checksum(wmr)) {
			fprintf(stderr, "Incorrect packet checksum, dropping packet\n");
			continue;
		}

		DEBUG_MSG("Packet 0x%02X (%u bytes)\n", wmr->packet_type, wmr->packet_len);
		printf("%li {\n", (long)parse_packet_time(wmr));
		dispatch_packet(wmr);
		printf("}\n\n");

		free(wmr->packet);

		if (exit_flag) {
			wmr_close(wmr);
			exit(0);
		}
	}
}


/******************** heartbeat timer ********************/


static void
alrm_handler(int signum) {
	send_heartbeat(wmr_global);
}


static void
setup_timer() {
	struct sigaction sa;
	sa.sa_handler = alrm_handler;
	sa.sa_flags = SA_RESTART;
	sigaction(SIGALRM, &sa, NULL); // TODO

	struct itimerval itval;
	itval.it_interval.tv_sec = HEARTBEAT_INTERVAL;
	itval.it_value.tv_sec = HEARTBEAT_INTERVAL;
	itval.it_interval.tv_usec = itval.it_value.tv_usec = 0;

	setitimer(ITIMER_REAL, &itval, NULL);
}



static void
stop_timer() {
	// TODO
}


/******************** interface  ********************/


struct wmr200 *
wmr_open() {
	struct wmr200 *wmr = malloc_safe(sizeof(struct wmr200));

	wmr->dev = hid_open(WMR200_VID, WMR200_PID, NULL);
	if (wmr->dev == NULL) {
		DEBUG_MSG("hid_open(): cannot connect to WRM200\n");
		return NULL;
	}

	wmr->packet = NULL;
	wmr->buf_avail = wmr->buf_pos = 0;
	wmr->handler = NULL;

	wmr_global = wmr; // TODO

	// some kind of wake-up command
	uchar abracadabra[8] = { 0x20, 0x00, 0x08, 0x01, 0x00, 0x00, 0x00, 0x00 };
	if (hid_write(wmr->dev, abracadabra, 8) != 8) {
		DEBUG_MSG("Cannot initialize communication with WMR200\n");
		return NULL;
	}

	send_heartbeat(wmr);
	setup_timer();

	return wmr;
}


void
wmr_close(struct wmr200 *wmr) {
	stop_timer();

	if (wmr->dev != NULL) {
		send_cmd_frame(wmr, COMMUNICATION_STOP);
		hid_exit();
	}
}


void
wmr_init() {
	hid_init();
}


void
wmr_end() {
	 // TODO unregister handlers, stop the timer
}


void
wmr_main_loop(struct wmr200 *wmr) {
	main_loop(wmr);
}


void
wmr_set_handler(struct wmr200 *wmr, void (*handler)(struct wmr_reading *)) {
	struct wmr_handler *wh = malloc_safe(sizeof(struct wmr_handler));
	wh->handler = handler;
	wh->next = wmr->handler;
	wmr->handler = wh;
}
