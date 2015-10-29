/**
 *  logger.c:
 *  Very simple WMR200 datalogger
 *
 *  Copyright (c) 2015 David Čepelík <cepelik@gymlit.cz>
 *
 *  This program may be licensed under GNU GPL version 2 or 3.
 */


#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <time.h>

#include <hidapi.h>


typedef unsigned int		uint;
typedef unsigned char		uchar;


#define WMR200_VID		0x0FDE
#define WMR200_PID		0xCA01

#define HEARTBEAT		0xD0
#define HISTORIC_DATA_NOTIF	0xD1
#define HISTORIC_DATA		0xD2
#define WIND_DATA		0xD3
#define RAIN_DATA		0xD4
#define UVI_DATA		0xD5
#define BARO_DATA		0xD6
#define TEMP_HUMID_DATA		0xD7
#define STATUS_DATA		0xD9
#define REQUEST_HISTORIC_DATA	0xDA
#define LOGGER_DATA_ERASE	0xDB
#define COMMUNICATION_STOP	0xDF

#define FRAME_SIZE		8	// Bytes
#define HEARTBEAT_PERIOD	30	// seconds
#define MAX_EXT_SENSORS		10

#define SIGN_POSITIVE		0x0
#define SIGN_NEGATIVE		0x8

#define NTH_BIT(n, val)		(((val) >> (n)) & 1)
#define HIGH(b)			LOW((b) >> 4)
#define LOW(b)			((b) &  0xF)

#ifdef  DEBUG
#define DEBUG_MSG(...)		fprintf(stderr, __VA_ARGS__)
#else
#define DEBUG_MSG(...)
#endif


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


hid_device *dev;

uchar buf[FRAME_SIZE];
uint buf_avail;
uint buf_pos;

uchar *packet;
uchar packet_type;
uint packet_len;


void
send_cmd_frame(uchar cmd) {
	uchar data[FRAME_SIZE] = { 0x01, cmd };
	int ret = hid_write(dev, data, FRAME_SIZE);

	if (ret != FRAME_SIZE) {
		fprintf(stderr, "hid_write: cannot send %02x command frame, return was %i", cmd, ret);
		exit(1);
	}
}



void set_heartbeat_timer();

void
hearbeat(int sig_num) {
	DEBUG_MSG("Sending heartbeat\n");
	send_cmd_frame(HEARTBEAT);

	set_heartbeat_timer();
}


void
set_heartbeat_timer() {
	signal(SIGALRM, hearbeat);
	alarm(HEARTBEAT_PERIOD);
}


void
cancel_heartbeat_timer() {
	alarm(0);
}


uchar
read_byte() {
	if (buf_avail == 0) {
		int ret = hid_read(dev, buf, FRAME_SIZE);

		if (ret != FRAME_SIZE) {
			fprintf(stderr, "hid_read: cannot read frame, return was %i\n", ret);
			exit(1);
		}

		buf_avail = buf[0];
		buf_pos = 1;
	}

	buf_avail--;
	return buf[buf_pos++];
}


void
connect() {
	dev = hid_open(WMR200_VID, WMR200_PID, NULL);

	if (dev == NULL) {
		fprintf(stderr, "hid_open: cannot connect to WRM200\n");
		exit(1);
	}

	// abracadabra
	uchar init[8] = { 0x20, 0x00, 0x08, 0x01, 0x00, 0x00, 0x00, 0x00 };
	if (hid_write(dev, init, 8) != 8) {
		fprintf(stderr, "Cannot initialize communication with WMR200\n");
		exit(1);
	}

	// todo
	uchar start[8] = { 0x01, 0xD0, 0x08, 0x01, 0x00, 0x00, 0x00, 0x00 };
	if (hid_write(dev, start, 8) != 8) {
		fprintf(stderr, "Cannot initialize communication with WMR200\n");
		exit(1);
	}

	send_cmd_frame(HEARTBEAT);

	buf_avail = 0;
}


void
disconnect() {
	cancel_heartbeat_timer();

	if (dev != NULL) {
		send_cmd_frame(COMMUNICATION_STOP);
		hid_exit();
	}
}



time_t
parse_packet_time() {
	struct tm tm = {
		.tm_year	= (2000 + packet[6]) - 1900,
		.tm_mon		= packet[5],
		.tm_mday	= packet[4],
		.tm_hour	= packet[3],
		.tm_min		= packet[2],
		.tm_sec		= 0,
		.tm_isdst	= -1
	};

	return mktime(&tm);
}


void
process_wind_data(uchar *data, uint data_len) {
	uint wind_dir_flag	= LOW(data[7]);
	float wind_gust_speed	= (256 * LOW(data[10]) +      data[9])   / 10;
	float wind_avg_speed	= (256 * LOW(data[11]) + HIGH(data[10])) / 10;
	float wind_chill	= (data[12] - 32) / 1.8;


	// wind direction, wind gust speed, wind average speed and wind chill
	printf("\twind.dir: %s\n", WIND_DIRECTION[wind_dir_flag]);
	printf("\twind.gust_speed: %.2f\n", wind_gust_speed);
	printf("\twind.avg_speed: %.2f\n", wind_avg_speed);
	printf("\twind.chill: %.1f\n", wind_chill);
}


void
process_rain_data(uchar *data, uint data_len) {
	float rain_rate		= (256 * data[8]  +  data[7]) * 25.4;
	float rain_hour		= (256 * data[10] +  data[9]) * 25.4;
	float rain_24h		= (256 * data[12] + data[11]) * 25.4;
	float rain_accum 	= (256 * data[14] + data[13]) * 25.4;


	// rain rate, rain last hour, rain last 24 hours (excl. last hour)
	// and accumulated rain since 2007-01-01 12:00
	printf("\train.rate: %.2f\n", rain_rate);
	printf("\train.hour: %.2f\n", rain_hour);
	printf("\train.24h: %.2f\n", rain_24h);
	printf("\train.accum: %.2f\n", rain_accum);
}


void
process_uvi_data(uchar *data, uint data_len) {
	uint index = LOW(data[7]);
	printf("\tuvi.index: %u\n", index);
}


void
process_baro_data(uchar *data, uint data_len) {
	uint pressure		= 256 * LOW(data[8])  + data[7];
	uint alt_pressure	= 256 * LOW(data[10]) + data[9];
	uint forecast_flag	= HIGH(data[8]);


	// pressure, altitude pressure and forecast flag
	printf("\tbaro.pressure: %u\n", pressure);
	printf("\tbaro.alt_pressure: %u\n", alt_pressure);
	printf("\tbaro.forecast: %s\n", FORECAST[forecast_flag]);
}


void
process_temp_humid_data(uchar *data, uint data_len) {
	int sensor_id = data[7] & 0xF;

	// TODO
	if (sensor_id > 1) {
		fprintf(stderr, "Unknown sensor, ID: %i\n", sensor_id);
		exit(1);
	}

	char *sensor_name = (sensor_id == 1) ? "temp_hum_1" : "indoor";


	uint humidity   = data[10];
	uint heat_index = data[13];

	float temp = (256 * LOW(data[9]) + data[8]) / 10.0;
	if (HIGH(data[9]) == SIGN_NEGATIVE) temp = -temp;

	float dew_point = (256 * LOW(data[12]) + data[11]) / 10.0;
	if (HIGH(data[12]) == SIGN_NEGATIVE) dew_point = -dew_point;


	// temperature, dew point, humidity and heat index
	printf("\t%s.humidity: %u\n", sensor_name, humidity);
	printf("\t%s.heat_index: %u\n", sensor_name, heat_index);
	printf("\t%s.temp: %.1f\n", sensor_name, temp);
	printf("\t%s.dew_point: %.1f\n", sensor_name, dew_point);
}


void
process_status_data(uchar *data, uint data_len) {
	uint wind_bat_flag		= NTH_BIT(0, data[4]);
	uint temp_hum_bat_flag		= NTH_BIT(1, data[4]);
	uint rain_bat_flag		= NTH_BIT(4, data[5]);
	uint uv_bat_flag		= NTH_BIT(5, data[5]);

	uint wind_sensor_flag 		= NTH_BIT(0, data[2]);
	uint temp_hum_sensor_flag	= NTH_BIT(1, data[2]);
	uint rain_sensor_flag		= NTH_BIT(4, data[3]);
	uint uv_sensor_flag		= NTH_BIT(5, data[3]);

	uint rtc_signal_flag		= NTH_BIT(8, data[4]);


	// batteries
	printf("\tstatus.wind.bat: %s\n", LEVEL[wind_bat_flag]);
	printf("\tstatus.temp_hum.bat: %s\n", LEVEL[temp_hum_bat_flag]);
	printf("\tstatus.rain.bat: %s\n", LEVEL[rain_bat_flag]);
	printf("\tstatus.uv.bat: %s\n", LEVEL[uv_bat_flag]);

	// sensor states
	printf("\tstatus.wind.sensor: %s\n", STATUS[wind_sensor_flag]);
	printf("\tstatus.temp_hum.sensor: %s\n", STATUS[temp_hum_sensor_flag]);
	printf("\tstatus.rain.sensor: %s\n", STATUS[rain_sensor_flag]);
	printf("\tstatus.uv.sensor: %s\n", STATUS[uv_sensor_flag]);

	// real-time clock signal strength
	printf("\tstatus.rtc.signal: %s\n", LEVEL[rtc_signal_flag]);
}


void
process_historic_data(uchar *data, uint data_len) {
	process_rain_data(data, 0);
	process_wind_data(data + 13, 0);
	process_uvi_data(data + 20, 0);
	process_baro_data(data + 21, 0);
	process_temp_humid_data(data + 26, 0);

	uint ext_sensor_count = data[32];
	if (ext_sensor_count > MAX_EXT_SENSORS) {
		fprintf(stderr, "Too many external sensors\n");
		exit(1);
	}

	for (uint i = 0; i < ext_sensor_count; i++) {
		process_temp_humid_data(data + 33 + (7 * i), 0);
	}
}


uint
calc_packet_checksum() {
	uint sum = 0;
	for (uint i = 0; i < packet_len - 2; i++) {
		sum += packet[i];
	}

	return sum;
}


void dispatch_packet() {
	switch (packet_type) {
	case HISTORIC_DATA:
		process_historic_data(packet, packet_len);
		break;
	case WIND_DATA:
		process_wind_data(packet, packet_len);
		break;
	case RAIN_DATA:
		process_rain_data(packet, packet_len);
		break;
	case UVI_DATA:
		process_uvi_data(packet, packet_len);
		break;
	case BARO_DATA:
		process_baro_data(packet, packet_len);
		break;
	case TEMP_HUMID_DATA:
		process_temp_humid_data(packet, packet_len);
		break;
	case STATUS_DATA:
		process_status_data(packet, packet_len);
		break;
	}
}


void main_loop() {
	while (1) {
		packet_type = read_byte();

act_on_packet_type:
		switch (packet_type) {
		case HISTORIC_DATA_NOTIF:
			DEBUG_MSG("Data logger contains some unprocessed historic records\n");
			DEBUG_MSG("Issuing REQUEST_HISTORIC_DATA command\n");

			send_cmd_frame(REQUEST_HISTORIC_DATA);
			continue;

		case LOGGER_DATA_ERASE:
			DEBUG_MSG("Data logger database purge successful\n");
			continue;

		case COMMUNICATION_STOP:
			// ignore, sent as response to prev COMMUNICATION_STOP request
			DEBUG_MSG("Ignoring COMMUNICATION_STOP packet\n");
			break;
		}

		packet_len = read_byte();
		if (packet_len >= 0xD0 && packet_len <= 0xDF) {
			// this is a packet type mark, not packet length
			packet_type = packet_len;
			goto act_on_packet_type;
		}

		packet = malloc(packet_len);
		if (packet == NULL) {
			fprintf(stderr, "Cannot malloc %u bytes of memory\n", packet_len);
			exit(1);
		}

		packet[0] = packet_type;
		packet[1] = packet_len;

		for (uint i = 2; i < packet_len; i++) {
			packet[i] = read_byte();
		}

		uint recv_checksum
			= 256 * packet[packet_len - 1]
			+ packet[packet_len - 2];

		if (recv_checksum != calc_packet_checksum()) {
			fprintf(stderr, "Incorrect packet checksum, dropping packet\n");
			continue;
		}

		printf("# packet 0x%02X (%u bytes)\n", packet_type, packet_len);
		printf("%li {\n", (long)parse_packet_time());
		dispatch_packet();
		printf("}\n\n");

		free(packet);

		DEBUG_MSG("Processed %02x packet (%u bytes)\n", packet_type, packet_len);
	}
}	


void
cleanup(int sig_num) {
	disconnect();

	printf("\n\nDied on signal %i\n", sig_num);
	exit(0);
}


int
main(int argc, const char *argv[]) {
	signal(SIGINT, cleanup);
	signal(SIGTERM, cleanup);

	set_heartbeat_timer();

	connect();
	//send_cmd_frame(LOGGER_DATA_ERASE);
	main_loop();

	return (0);
}
