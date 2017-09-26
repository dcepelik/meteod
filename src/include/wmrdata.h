/*
 * wmrdata.h:
 * WMR data structures (encapsulation of readings) and utils
 *
 * This software may be freely used and distributed according to the terms
 * of the GNU GPL version 2 or 3. See LICENSE for more information.
 *
 * Copyright (c) 2015 David Čepelík <cepelik@gymlit.cz>
 */

#ifndef WMRDATA_H
#define	WMRDATA_H

#include "common.h"
#include "strbuf.h"

struct wmr200;


enum packet_type
{
	PACKET_ERASE_ACK = 0xDB,	/* data erase successful (after CMD_ERASE) */
	PACKET_HISTDATA_NOTIF = 0xD1,	/* historic data available notification */
	PACKET_STOP_ACK = 0xDF,		/* communication will stop (after CMD_STOP) */

	HISTORIC_DATA = 0xD2,
	WMR_WIND = 0xD3,
	WMR_RAIN = 0xD4,
	WMR_UVI	 = 0xD5,
	WMR_BARO = 0xD6,
	WMR_TEMP = 0xD7,
	WMR_STATUS = 0xD9,
	WMR_META = 0xFF,		/* system meta-packet */

	PACKET_TYPE_MAX
};

const char *packet_type_to_string(enum packet_type type);

struct wmr_wind
{
	const char *dir;	/* wind direction, see `struct wmr200.c' */
	float gust_speed;	/* gust speed, m/s */
	float avg_speed;	/* average speed, m/s */
	float chill;		/* TODO what's this? */
};

struct wmr_rain
{
	float rate;		/* immediate rain rate, mm/m^2 */
	float accum_hour;	/* rain last hour, mm/m^2 */
	float accum_24h;	/* rain 24 hours (without rain_hour), mm/m^2 */
	float accum_2007;	/* accum rain since 2007-01-01 12:00, mm/m^2 */
};

struct wmr_uvi
{
	uint_t index;		/* "UV index", value in range 0..15 */
};

struct wmr_baro
{
	uint_t pressure;	/* immediate pressure, hPa */
	uint_t alt_pressure;	/* TODO */
	const char *forecast;	/* name of "forecast icon", see `struct wmr200.c' */
};

struct wmr_temp
{
	uint_t sensor_id;	/* ID in range 0..MAX_EXT_SENSORS, 0 = console */
	uint_t humidity;	/* relative humidity, percent */
	uint_t heat_index;	/* value 0..4, 0 = undefined (temp too low) */
	float temp;		/* temperature, deg C */
	float dew_point;	/* dew point, deg C */
};

struct wmr_status
{
	/*
	 * Battery level strings (see `level_string' in `struct wmr200.c').
	 */
	const char *wind_bat;
	const char *temp_bat;
	const char *rain_bat;
	const char *uv_bat;

	/*
	 * Sensor status strings (see `status_string' in `struct wmr200.c').
	 */
	const char *wind_sensor;
	const char *temp_sensor;
	const char *rain_sensor;
	const char *uv_sensor;

	/*
	 * Real Time Clock (RTC) signal level (see `level_string' in `struct wmr200.c').
	 */
	const char *rtc_signal_level;
};

struct wmr_meta
{
	uint_t num_packets;
	uint_t num_failed;
	uint_t num_frames;
	float error_rate;
	ulong_t num_bytes;
	time_t latest_packet;
	time_t uptime;
};

struct wmr_reading
{
	uint_t type;
	time_t time;
	union
	{
		struct wmr_wind wind;
		struct wmr_rain rain;
		struct wmr_uvi uvi;
		struct wmr_baro baro;
		struct wmr_temp temp;
		struct wmr_status status;
		struct wmr_meta meta;
	};
};

const char *wmr_sensor_name(struct wmr_reading *reading);

struct wmr_latest_data
{
	struct wmr_reading wind;
	struct wmr_reading rain;
	struct wmr_reading uvi;
	struct wmr_reading baro;
	struct wmr_reading temp[10]; /* TODO */
	struct wmr_reading status;
	struct wmr_reading meta;
};

#endif
