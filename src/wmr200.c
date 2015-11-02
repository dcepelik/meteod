/*
 * wmr200.c:
 * Oregon Scientific WMR200 USB HID communication wrapper
 *
 * This software may be freely used and distributed according to the terms
 * of the GNU GPL version 2 or 3. See LICENSE for more information.
 *
 * Copyright (c) 2015 David Čepelík <cepelik@gymlit.cz>
 */


#include "wmr200.h"
#include "wmrdata.h"
#include "common.h"

#include <signal.h>
#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include <sys/time.h>


#define VENDOR_ID		0x0FDE
#define PRODUCT_ID		0xCA01

#define HEARTBEAT		0xD0
#define HISTORIC_DATA_NOTIF	0xD1
#define HISTORIC_DATA		0xD2
#define REQUEST_HISTORIC_DATA	0xDA
#define LOGGER_DATA_ERASE	0xDB
#define COMMUNICATION_STOP	0xDF

#define HEARTBEAT_INTERVAL	30 // s
#define MAX_EXT_SENSORS		10

#define SIGN_POSITIVE		0x0
#define SIGN_NEGATIVE		0x8


#define NTH_BIT(n, val)		(((val) >> (n)) & 1)
#define HIGH(b)			LOW((b) >> 4)
#define LOW(b)			((b) &  0xF)


struct wmr_handler {
	void (*handler)(wmr_reading *);
	struct wmr_handler *next;
};


wmr200 *wmr_global; // TODO


/******************** flag-to-string arrays ********************/


const char *LEVEL[] = {		// level (signal, battery)
	"ok",
	"low"
};

const char *STATUS[] = {
	"ok",
	"failed"
};

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


/******************** sending and receiving data ********************/


static uchar
read_byte(wmr200 *wmr) {
	if (wmr->buf_avail == 0) {
		int ret = hid_read(wmr->dev, wmr->buf, WMR200_FRAME_SIZE);

		if (ret != WMR200_FRAME_SIZE) {
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
send_cmd_frame(wmr200 *wmr, uchar cmd) {
	uchar data[WMR200_FRAME_SIZE] = { 0x01, cmd };
	int ret = hid_write(wmr->dev, data, WMR200_FRAME_SIZE);

	if (ret != WMR200_FRAME_SIZE) {
		fprintf(stderr, "hid_write: cannot send %02x command frame, return was %i", cmd, ret);
		exit(1);
	}
}


static void
send_heartbeat(wmr200 *wmr) {
	DEBUG_MSG("Sending heartbeat to WMR200\n");
	send_cmd_frame(wmr, HEARTBEAT);
}


/******************** data processing ********************/


static time_t
get_reading_time_from_packet(wmr200 *wmr) {
	struct tm time = {
		.tm_year	= (2000 + wmr->packet[6]) - 1900,
		.tm_mon		= wmr->packet[5],
		.tm_mday	= wmr->packet[4],
		.tm_hour	= wmr->packet[3],
		.tm_min		= wmr->packet[2],
		.tm_sec		= 0,
		.tm_isdst	= -1
	};

	return mktime(&time);
}


static void
invoke_handlers(wmr200 *wmr, wmr_reading *reading) {
	reading->time = get_reading_time_from_packet(wmr);

	struct wmr_handler *handler = wmr->handler;
	while (handler != NULL) {
		handler->handler(reading);
		handler = handler->next;
	}
}


static void
process_wind_data(wmr200 *wmr, uchar *data) {
	uint dir_flag		= LOW(data[7]);
	float gust_speed	= (256 * LOW(data[10]) +      data[9])   / 10.0;
	float avg_speed		= ( 16 * LOW(data[11]) + HIGH(data[10])) / 10.0;
	float chill		= data[12]; // (data[12] - 32) / 1.8;

	invoke_handlers(wmr, &(wmr_reading) {
		.type = WMR_WIND,
		.wind = {
			.dir = WIND_DIRECTION[dir_flag],
			.gust_speed = gust_speed,
			.avg_speed = avg_speed,
			.chill = chill
		}
	});
}


static void
process_rain_data(wmr200 *wmr, uchar *data) {
	float rate		= (256 * data[8]  +  data[7]) * 25.4;
	float accum_hour	= (256 * data[10] +  data[9]) * 25.4;
	float accum_24h		= (256 * data[12] + data[11]) * 25.4;
	float accum_2007 	= (256 * data[14] + data[13]) * 25.4;

	invoke_handlers(wmr, &(wmr_reading) {
		.type = WMR_RAIN,
		.rain = {
			.rate = rate,
			.accum_hour = accum_hour,
			.accum_24h = accum_24h,
			.accum_2007 = accum_2007
		}
	});
}


static void
process_uvi_data(wmr200 *wmr, uchar *data) {
	uint index = LOW(data[7]);

	invoke_handlers(wmr, &(wmr_reading) {
		.type = WMR_UVI,
		.uvi = {
			.index = index
		}
	});
}


static void
process_baro_data(wmr200 *wmr, uchar *data) {
	uint pressure		= 256 * LOW(data[8])  + data[7];
	uint alt_pressure	= 256 * LOW(data[10]) + data[9];
	uint forecast_flag	= HIGH(data[8]);

	invoke_handlers(wmr, &(wmr_reading) {
		.type = WMR_BARO,
		.baro = {
			.pressure = pressure,
			.alt_pressure = alt_pressure,
			.forecast = FORECAST[forecast_flag]
		}
	});
}


static void
process_temp_data(wmr200 *wmr, uchar *data) {
	int sensor_id = LOW(data[7]);

	// TODO
	if (sensor_id > 1) {
		fprintf(stderr, "Unknown sensor, ID: %i\n", sensor_id);
		exit(1);
	}

	uint humidity   = data[10];
	uint heat_index = data[13];

	float temp = (256 * LOW(data[9]) + data[8]) / 10.0;
	if (HIGH(data[9]) == SIGN_NEGATIVE) temp = -temp;

	float dew_point = (256 * LOW(data[12]) + data[11]) / 10.0;
	if (HIGH(data[12]) == SIGN_NEGATIVE) dew_point = -dew_point;


	invoke_handlers(wmr, &(wmr_reading) {
		.type = WMR_TEMP,
		.temp = {
			.humidity = humidity,
			.heat_index = heat_index,
			.temp = temp,
			.dew_point = dew_point,
			.sensor_id = sensor_id
		}
	});
}


static void
process_status_data(wmr200 *wmr, uchar *data) {
	uint wind_bat_flag		= NTH_BIT(0, data[4]);
	uint temp_bat_flag		= NTH_BIT(1, data[4]);
	uint rain_bat_flag		= NTH_BIT(4, data[5]);
	uint uv_bat_flag		= NTH_BIT(5, data[5]);

	uint wind_sensor_flag 		= NTH_BIT(0, data[2]);
	uint temp_sensor_flag		= NTH_BIT(1, data[2]);
	uint rain_sensor_flag		= NTH_BIT(4, data[3]);
	uint uv_sensor_flag		= NTH_BIT(5, data[3]);

	uint rtc_signal_flag		= NTH_BIT(8, data[4]);


	invoke_handlers(wmr, &(wmr_reading) {
		.type = WMR_STATUS,
		.status = {
			.wind_bat = LEVEL[wind_bat_flag],
			.temp_bat = LEVEL[temp_bat_flag],
			.rain_bat = LEVEL[rain_bat_flag],
			.uv_bat = LEVEL[uv_bat_flag],

			.wind_sensor = STATUS[wind_sensor_flag],
			.temp_sensor = STATUS[temp_sensor_flag],
			.rain_sensor = STATUS[rain_sensor_flag],
			.uv_sensor = STATUS[uv_sensor_flag],

			.rtc_signal_level = LEVEL[rtc_signal_flag]
		}
	});
}


static void
process_historic_data(wmr200 *wmr, uchar *data) {
	process_rain_data(wmr, data);
	process_wind_data(wmr, data + 13);
	process_uvi_data(wmr, data + 20);
	process_baro_data(wmr, data + 21);
	process_temp_data(wmr, data + 26);

	uint ext_sensor_count = data[32];
	if (ext_sensor_count > MAX_EXT_SENSORS) {
		fprintf(stderr, "Too many external sensors\n");
		exit(1);
	}

	for (uint i = 0; i < ext_sensor_count; i++) {
		process_temp_data(wmr, data + 33 + (7 * i));
	}
}


/******************** packet processing ********************/


static uint
verify_packet(wmr200 *wmr) {
	if (wmr->packet_len <= 2) {
		return (-1);
	}

	uint sum = 0;
	for (uint i = 0; i < wmr->packet_len - 2; i++) {
		sum += wmr->packet[i];
	}

	uint checksum = 256 * wmr->packet[wmr->packet_len - 1]
		            + wmr->packet[wmr->packet_len - 2];

	if (sum != checksum) {
		return (-1);
	}

	// verify packet_len

	return (0);
}


static void
dispatch_packet(wmr200 *wmr) {
	switch (wmr->packet_type) {
	case HISTORIC_DATA:
		process_historic_data(wmr, wmr->packet);
		break;

	case WMR_WIND:
		process_wind_data(wmr, wmr->packet);
		break;

	case WMR_RAIN:
		process_rain_data(wmr, wmr->packet);
		break;

	case WMR_UVI:
		process_uvi_data(wmr, wmr->packet);
		break;

	case WMR_BARO:
		process_baro_data(wmr, wmr->packet);
		break;

	case WMR_TEMP:
		process_temp_data(wmr, wmr->packet);
		break;

	case WMR_STATUS:
		process_status_data(wmr, wmr->packet);
		break;

	default:
		DEBUG_MSG("Ignoring unknown packet %u\n", wmr->packet_type);
	}
}


static void
main_loop(wmr200 *wmr) {
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
			// this is packet type mark, not packet length
			wmr->packet_type = wmr->packet_len;
			goto act_on_packet_type;
		}

		wmr->packet = malloc_safe(wmr->packet_len);
		wmr->packet[0] = wmr->packet_type;
		wmr->packet[1] = wmr->packet_len;

		for (uint i = 2; i < wmr->packet_len; i++) {
			wmr->packet[i] = read_byte(wmr);
		}

		if (verify_packet(wmr) != 0) {
			fprintf(stderr, "Packet incorrect, dropping\n");
			continue;
		}

		DEBUG_MSG("Packet 0x%02X (%u bytes)\n", wmr->packet_type, wmr->packet_len);
		dispatch_packet(wmr);

		free(wmr->packet);
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


wmr200 *
wmr_open() {
	wmr200 *wmr = malloc_safe(sizeof(wmr200));

	wmr->dev = hid_open(VENDOR_ID, PRODUCT_ID, NULL);
	if (wmr->dev == NULL) {
		DEBUG_MSG("hid_open(): cannot connect to WRM200\n");
		return NULL;
	}

	wmr->packet = NULL;
	wmr->buf_avail = wmr->buf_pos = 0;
	wmr->handler = NULL;

	wmr_global = wmr; // TODO

	// some kind of a wake-up command
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
wmr_close(wmr200 *wmr) {
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
wmr_main_loop(wmr200 *wmr) {
	main_loop(wmr);
}


void
wmr_set_handler(wmr200 *wmr, void (*handler)(wmr_reading *)) {
	struct wmr_handler *wh = malloc_safe(sizeof(struct wmr_handler));
	wh->handler = handler;
	wh->next = wmr->handler;
	wmr->handler = wh;
}



