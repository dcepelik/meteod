#ifndef WMR200_H
#define WMR200_H

#include <hidapi.h>
#include "common.h"
#include "macros.h"


struct wmr_reading;


struct wmr_handler {
	void (*handler)(struct wmr_reading *);
	struct wmr_handler *next;
};


struct wmr200 {
	hid_device *dev;			// HIDAPI device handle

	uchar buf[FRAME_SIZE];			// receive buffer
	uint buf_avail;
	uint buf_pos;

	uchar *packet;				// packet being processed
	uchar packet_type;
	uint packet_len;

	struct wmr_handler *handler;		// handlers
};


void wmr_init();


void wmr_end();


struct wmr200 *wmr_open();


void wmr_close(struct wmr200 *wmr);


void wmr_main_loop(struct wmr200 *wmr);


void wmr_set_handler(struct wmr200 *wmr, void (*handler)(struct wmr_reading *));


struct wmr_wind_reading {
	const char *dir;	// wind direction, see `wmr200.c'
	float gust_speed;	// gust speed, m/s
	float avg_speed;	// average speed, m/s
	float chill;		// TODO
};


struct wmr_rain_reading {
	float rate;		// immediate rain rate, mm/m^2
	float accum_hour;	// rain last hour, mm/m^2
	float accum_24h;	// rain 24 hours (without rain_hour), mm/m^2
	float accum_2007;	// accumulated rain since 2007-01-01 12:00, mm/m^2
};


struct wmr_uvi_reading {
	uint index;		// "UV index", value in range 0..15
};


struct wmr_baro_reading {
	uint pressure;		// immediate pressure, hPa
	uint alt_pressure;	// TODO
	const char *forecast;	// name of "forecast icon", see `wmr200.c'
};


struct wmr_temp_reading {
	uint sensor_id;		// ID in range 0..MAX_EXT_SENSORS, 0 = console
	uint humidity;		// relative humidity, percent
	uint heat_index;	// value 0..4, 0 = undefined (temp too low)
	float temp;		// temperature, deg C
	float dew_point;	// dew point, deg C
};


struct wmr_status_reading {
	const char *wind_bat;		// battery levels, see `wmr200.c'
	const char *temp_bat;
	const char *rain_bat;
	const char *uv_bat;

	const char *wind_sensor;	// sensor states, see `wmr200.c'
	const char *temp_sensor;
	const char *rain_sensor;
	const char *uv_sensor;

	const char *rtc_signal_level;	// signal level of the RTC
};


struct wmr_reading {
	uint type;
	time_t time;
	union {
		struct wmr_wind_reading wind;
		struct wmr_rain_reading rain;
		struct wmr_uvi_reading uvi;
		struct wmr_baro_reading baro;
		struct wmr_temp_reading temp;
		struct wmr_status_reading status;
	};
};


#endif
