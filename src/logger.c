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


typedef unsigned int		uint;
typedef unsigned char		uchar;



hid_device *dev;

uchar buf[FRAME_SIZE];
uint buf_avail;
uint buf_pos;

uchar *packet_data;
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
	printf("Reading byte\n");
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
		do {
			send_cmd_frame(COMMUNICATION_STOP);
		} while (read_byte() != COMMUNICATION_STOP);

		printf("Communication closed cleanly.");
		hid_close(dev);
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

}



void
process_uvi_data() {

}



void
process_baro_data() {

}



void
process_temp_humid_data() {
	
}



void
process_status_data() {
	check_packet_len(8);

	char *flags[2];
	flags[0] = "ok";
	flags[1] = "low";

	printf("status\trtc\tsignal:%s\n", flags[packet_data[4] & 128]);
	printf("status\twind\tsignal:%s\n", flags[packet_data[2] & 1]);
}



uint
calc_packet_checksum() {
	uint sum = 0;
	for (uint i = 0; i < packet_len - 2; i++) {
		sum += packet_data[i];
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
			printf("Data logger contains some unprocessed history records\n");
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


		packet_data = malloc(packet_len);
		if (packet_data == NULL) {
			fprintf(stderr, "Cannot malloc %u bytes of memory\n", packet_len);
			exit(1);
		}

		packet_data[0] = packet_type;
		packet_data[1] = packet_len;

		for (uint i = 2; i < packet_len; i++) {
			packet_data[i] = read_byte();
		}

		uint recv_checksum
			= 256 * packet_data[packet_len - 1]
			+ packet_data[packet_len - 2];

		if (recv_checksum != calc_packet_checksum()) {
			fprintf(stderr, "Incorrect packet checksum, dropping packet\n");
			continue;
		}

		dispatch_packet();
		free(packet_data);

		printf("Processed %02x packet, %i bytes.\n", packet_type, packet_len);
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

	//set_heartbeat_timer();

	connect();
	send_cmd_frame(LOGGER_DATA_ERASE);
	read_packets();

	return (0);
}
