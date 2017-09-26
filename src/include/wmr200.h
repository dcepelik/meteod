#ifndef WMR200_H
#define	WMR200_H

#include "common.h"

#include <stdio.h>
#include <hidapi.h>
#include <pthread.h>


#define	WMR200_MAX_TEMP_SENSORS		10

struct wmr200;

/*
 * Type of a packet received from the station.
 */
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

/*
 * Convert packet type to string name.
 */
const char *packet_type_to_string(enum packet_type type);

/*
 * Wind reading.
 */
struct wmr_wind
{
	const char *dir;	/* wind direction, see `struct wmr200.c' */
	float gust_speed;	/* gust speed, m/s */
	float avg_speed;	/* average speed, m/s */
	float chill;		/* TODO what's this? */
};

/*
 * Rain reading.
 */
struct wmr_rain
{
	float rate;		/* immediate rain rate, mm/m^2 */
	float accum_hour;	/* rain last hour, mm/m^2 */
	float accum_24h;	/* rain 24 hours (without rain_hour), mm/m^2 */
	float accum_2007;	/* accum rain since 2007-01-01 12:00, mm/m^2 */
};

/*
 * UV index reading.
 */
struct wmr_uvi
{
	uint_t index;		/* "UV index", value in range 0..15 */
};

/*
 * Barometric reading.
 */
struct wmr_baro
{
	uint_t pressure;	/* immediate pressure, hPa */
	uint_t alt_pressure;	/* TODO */
	const char *forecast;	/* name of "forecast icon", see `struct wmr200.c' */
};

/*
 * Temperature/humidity reading.
 */
struct wmr_temp
{
	uint_t sensor_id;	/* ID in range 0..MAX_EXT_SENSORS, 0 = console */
	uint_t humidity;	/* relative humidity, percent */
	uint_t heat_index;	/* value 0..4, 0 = undefined (temp too low) */
	float temp;		/* temperature, deg C */
	float dew_point;	/* dew point, deg C */
};

/*
 * Status reading.
 */
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

/*
 * An aritificial meta-reading produced by this module.
 */
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

/*
 * Encapsulation of a reading.
 */
struct wmr_reading
{
	byte_t type;
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

/*
 * For a given reading, return the name of the sensor where the reading
 * has originated.
 */
const char *wmr_sensor_name(struct wmr_reading *reading);

/*
 * A structure to hold latest data, i.e. the latest reading of every
 * possible kind. And for each temperature sensor, too.
 */
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

/*
 * Logger function prototype.
 */
typedef void wmr_logger_t(struct wmr200 *wmr, struct wmr_reading *reading, void *arg);

/*
 * Error handler prototype.
 */
typedef void wmr_err_handler_t(struct wmr200 *wmr, void *arg);

/*
 * Initialize the WMR200 module.
 */
void wmr_init();

/*
 * Dispose global resources held by WMR200 module.
 */
void wmr_end();

/*
 * Attempt to establish connection to a WMR200.
 *
 * Return value:
 *	If successful, returns a device handle.
 *	Returns NULL on failure.
 */
struct wmr200 *wmr_open(void);

/*
 * Close connection to the specified device.
 */
void wmr_close(struct wmr200 *wmr);

/*
 * Start communication with @wmr. Heartbeats will be sent to the
 * station and readings will be received.
 *
 * Once communication is started, received readings will be passed
 * to registered loggers. Also, when an error occurs,  error handler
 * will be invoked.
 *
 * Return value:
 *	If successful, returns zero.
 *	Negative value is returned on failure.
 */
int wmr_start(struct wmr200 *wmr);

/*
 * End communication with @wmr. The station will switch to data
 * logging mode immediately.
 */
void wmr_stop(struct wmr200 *wmr);

/*
 * Register a reading-logging callback @logger with @wmr.
 *
 * When a reading is received, the registered callback @logger will be
 * invoked and the reading will be passed to it. It is possible to pass
 * an extra argument @arg to @logger.
 */
void wmr_register_logger(struct wmr200 *wmr, wmr_logger_t *logger, void *arg);

/*
 * Register error handler @handler with @wmr. Extra argument @arg will
 * be passed to @handler upon invocation.
 *
 * The error handler is called when invalid data is received, when
 * communication errors occur, etc., i.e. when serious errors occur.
 * Most of these cannot be easily recovered from. Therefore, the
 * handleris required to call wmr_stop and wmr_close on @wmr and
 * behaviour is undefined otherwise.
 *
 * As a "soft-fail" alternative, consider reconnecting to the station.
 * in the error handling code.
 */
void wmr_set_error_handler(struct wmr200 *wmr, wmr_err_handler_t *handler, void *arg);

#endif
