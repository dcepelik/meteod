#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>

#include <hidapi.h>


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

#define SIGN_POSITIVE		0x0
#define SIGN_NEGATIVE		0x8

const char *LEVEL_FLAGS[] = { "ok", "low" };
const char *SENSOR_FLAGS[] = { "ok", "failed" };

const char *FORECAST_FLAGS[] = {
	"partly_cloudy-day",
	"rainy",
	"cloudy",
	"sunny",
	"clear",
	"snowy",
	"partly_cloudy-night"
};


#define NTH_BIT(n, val)		(((val) >> (n)) & 1)
#define HIGH(b)			LOW((b) >> 4)
#define LOW(b)			((b) &  0xF)


typedef unsigned int		uint;
typedef unsigned char		uchar;



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
	printf("Heartbeat\n");
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


void
check_packet_len(uint exp_len) {
	if (packet_len != exp_len) {
		fprintf(
			stderr,
			"Packet was %i bytes long, but %i bytes were expected\n",
			packet_len,
			exp_len
		);

		exit(1);
	}
}


void
process_historic_data() {
}


void
process_wind_data() {

}


void
process_rain_data() {
	check_packet_len(22);

	float rain_rate		= (256 * packet[8]  +  packet[7]) * 25.4;
	float rain_hour		= (256 * packet[10] +  packet[9]) * 25.4;
	float rain_24h		= (256 * packet[12] + packet[11]) * 25.4;
	float rain_accum 	= (256 * packet[14] + packet[13]) * 25.4;


	// rain rate, rain last hour, rain last 24 hours (excl. last hour)
	// and accumulated rain since 2007-01-01 12:00
	printf("rain.rate: %.2f\n", rain_rate);
	printf("rain.hour: %.2f\n", rain_hour);
	printf("rain.24h: %.2f\n", rain_24h);
	printf("rain.accum: %.2f\n", rain_accum);
}


void
process_uvi_data() {
	check_packet_len(10);

	uint index = LOW(byte[7]);
	printf("uvi.index: %u\n", index);
}


void
process_baro_data() {
	check_packet_len(13);

	uint pressure		= 256 * LOW(packet[8])  + packet[7];
	uint alt_pressure	= 256 * LOW(packet[10]) + packet[9];
	uint forecast_flag	= HIGH(packet[8]);


	// pressure, altitude pressure and forecast flag
	printf("baro.pressure: %u\n", pressure);
	printf("baro.alt_pressure: %u\n", alt_pressure);
	printf("baro.forecast: %s\n", FORECAST_FLAGS[forecast_flag]);
}


void
process_temp_humid_data() {
	check_packet_len(16);

	int sensor_id = packet[7] & 0xF;

	// TODO
	if (sensor_id > 1) {
		fprintf(stderr, "Unknown sensor, ID: %i\n", sensor_id);
		exit(1);
	}

	char *sensor_name = (sensor_id == 1) ? "temp_hum_1" : "indoor";


	uint humidity   = packet[10];
	uint heat_index = packet[13];

	float temp = (256 * LOW(packet[9]) + packet[8]) / 10.0;
	if (HIGH(packet[9]) == SIGN_NEGATIVE) temp = -temp;

	float dew_point = (256 * LOW(packet[12]) + packet[11]) / 10.0;
	if (HIGH(packet[12]) == SIGN_NEGATIVE) dew_point = -dew_point;


	// temperature, dew point, humidity and heat index
	printf("%s.humidity: %u\n", sensor_name, humidity);
	printf("%s.heat_index: %u\n", sensor_name, heat_index);
	printf("%s.temp: %.1f\n", sensor_name, temp);
	printf("%s.dew_point: %.1f\n", sensor_name, dew_point);
}


void
process_status_data() {
	check_packet_len(8);

	uint wind_bat_flag		= NTH_BIT(0, packet[4]);
	uint temp_hum_bat_flag		= NTH_BIT(1, packet[4]);
	uint rain_bat_flag		= NTH_BIT(4, packet[5]);
	uint uv_bat_flag		= NTH_BIT(5, packet[5]);

	uint wind_sensor_flag 		= NTH_BIT(0, packet[2]);
	uint temp_hum_sensor_flag	= NTH_BIT(1, packet[2]);
	uint rain_sensor_flag		= NTH_BIT(4, packet[3]);
	uint uv_sensor_flag		= NTH_BIT(5, packet[3]);

	uint rtc_signal_flag		= NTH_BIT(8, packet[4]);


	// batteries
	printf("status.wind.bat: %s\n", LEVEL_FLAGS[wind_bat_flag]);
	printf("status.temp_hum.bat: %s\n", LEVEL_FLAGS[temp_hum_bat_flag]);
	printf("status.rain.bat: %s\n", LEVEL_FLAGS[rain_bat_flag]);
	printf("status.uv.bat: %s\n", LEVEL_FLAGS[uv_bat_flag]);

	// sensor states
	printf("status.wind.sensor: %s\n", SENSOR_FLAGS[wind_sensor_flag]);
	printf("status.temp_hum.sensor: %s\n", SENSOR_FLAGS[temp_hum_sensor_flag]);
	printf("status.rain.sensor: %s\n", SENSOR_FLAGS[rain_sensor_flag]);
	printf("status.uv.sensor: %s\n", SENSOR_FLAGS[uv_sensor_flag]);

	// real-time clock signal strength
	printf("status.rtc.signal: %s\n", LEVEL_FLAGS[rtc_signal_flag]);
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
		process_historic_data();
		break;
	case WIND_DATA:
		process_wind_data();
		break;
	case RAIN_DATA:
		process_rain_data();
		break;
	case UVI_DATA:
		process_uvi_data();
		break;
	case BARO_DATA:
		process_baro_data();
		break;
	case TEMP_HUMID_DATA:
		process_temp_humid_data();
		break;
	case STATUS_DATA:
		process_status_data();
		break;
	}
}


void read_packets() {
	while (1) {
		packet_type = read_byte();

act_on_packet_type:
		switch (packet_type) {
		case HISTORIC_DATA_NOTIF:
			printf("Data logger contains some unprocessed historic records\n");
			printf("Issuing REQUEST_HISTORIC_DATA command\n");

			send_cmd_frame(REQUEST_HISTORIC_DATA);
			continue;

		case LOGGER_DATA_ERASE:
			printf("Data logger database purge successful\n");
			continue;

		case COMMUNICATION_STOP:
			// ignore, sent as response to prev COMMUNICATION_STOP request
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

		dispatch_packet();
		free(packet);
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
	send_cmd_frame(LOGGER_DATA_ERASE);
	read_packets();

	return (0);
}
