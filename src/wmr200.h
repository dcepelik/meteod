#ifndef WMR200_H
#define WMR200_H

#include "wmr.h"
#include "common.h"

#include <stdio.h>
#include <hidapi.h>


typedef struct {
	const char *dir;	// wind direction, see `wmr200.c'
	float gust_speed;	// gust speed, m/s
	float avg_speed;	// average speed, m/s
	float chill;		// TODO
} wmr_wind;


typedef struct {
	float rate;		// immediate rain rate, mm/m^2
	float accum_hour;	// rain last hour, mm/m^2
	float accum_24h;	// rain 24 hours (without rain_hour), mm/m^2
	float accum_2007;	// accumulated rain since 2007-01-01 12:00, mm/m^2
} wmr_rain;


typedef struct {
	uint index;		// "UV index", value in range 0..15
} wmr_uvi;


typedef struct {
	uint pressure;		// immediate pressure, hPa
	uint alt_pressure;	// TODO
	const char *forecast;	// name of "forecast icon", see `wmr200.c'
} wmr_baro;


typedef struct {
	uint sensor_id;		// ID in range 0..MAX_EXT_SENSORS, 0 = console
	uint humidity;		// relative humidity, percent
	uint heat_index;	// value 0..4, 0 = undefined (temp too low)
	float temp;		// temperature, deg C
	float dew_point;	// dew point, deg C
} wmr_temp;


typedef struct {
	const char *wind_bat;		// battery levels, see `wmr200.c'
	const char *temp_bat;
	const char *rain_bat;
	const char *uv_bat;

	const char *wind_sensor;	// sensor states, see `wmr200.c'
	const char *temp_sensor;
	const char *rain_sensor;
	const char *uv_sensor;

	const char *rtc_signal_level;	// signal level of the RTC
} wmr_status;


typedef struct {
	uint type;
	time_t time;
	union {
		wmr_wind wind;
		wmr_rain rain;
		wmr_uvi uvi;
		wmr_baro baro;
		wmr_temp temp;
		wmr_status status;
	};
} wmr_reading;


struct wmr_handler;


typedef struct {
	hid_device *dev;			// HIDAPI device handle

	uchar buf[FRAME_SIZE];			// receive buffer
	uint buf_avail;
	uint buf_pos;

	uchar *packet;				// packet being processed
	uchar packet_type;
	uint packet_len;

	struct wmr_handler *handler;		// handlers
} wmr200;


void wmr_init();


void wmr_end();


wmr200 *wmr_open();


void wmr_close(wmr200 *wmr);


void wmr_main_loop(wmr200 *wmr);


void wmr_set_handler(wmr200 *wmr, void (*handler)(wmr_reading *));


#endif
