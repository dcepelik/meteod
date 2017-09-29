/*
 * Log readings to round-robin database (RRD) files.
 * Copyright (c) 2015-2017 David Čepelík <d@dcepelik.cz>.
 */

#include "common.h"
#include "log.h"
#include "rrd-logger.h"

#include <assert.h>
#include <limits.h>
#include <rrd.h>
#include <time.h>

char path_buf[PATH_MAX];	/* static path buffer */

/*
 * Paste paths p1 and p2.
 *
 * NOTE: The resulting string is alloc'd in a static buffer, don't pass it
 *       to free(3).
 */
static char *paste_path(char *p1, char *p2)
{
	assert(snprintf(path_buf, sizeof(path_buf), "%s/%s", p1, p2) >= 0);
	return path_buf;
}

/*
 * Update an RRD database file found whose path relative to configured
 * root is @rel_path.
 *
 * NOTE: One parameter is implicit: the data which should be written to the
 *       database file, which is contained in @logger->data.
 */
static void update(struct rrd_logger *logger, char *rel_path)
{
	int ret;
	char *cpy;

	/* TODO: insert reading time instead of current time */
	cpy = strbuf_strcpy(&logger->data);
	strbuf_reset(&logger->data);
	strbuf_printf(&logger->data, "%li:%s", time(NULL), cpy);
	free(cpy);

	char *update_params[] = {
		"rrdupdate",
		paste_path(logger->cfg.rrd_root, rel_path),
		strbuf_get_string(&logger->data),
		NULL
	};

	ret = rrd_update(ARRAY_SIZE(update_params) - 1, update_params);
	if (ret != 0) {
		log_error("rrd_update: %s", rrd_get_error()); /* TODO quit */
		rrd_clear_error();
	}
}

static void log_wind(struct rrd_logger *logger, struct wmr_wind *wind)
{
	strbuf_reset(&logger->data);
	strbuf_printf(
		&logger->data,
		"%.1f:%.1f",
		wind->avg_speed,
		wind->gust_speed);

	update(logger, logger->cfg.wind_rrd);
}

static void log_rain(struct rrd_logger *logger, struct wmr_rain *rain)
{
	strbuf_reset(&logger->data);
	strbuf_printf(
		&logger->data,
		"%.1f:%.0f",
		rain->rate,
		rain->accum_2007);

	update(logger, logger->cfg.rain_rrd);
}

static void log_uvi(struct rrd_logger *logger, struct wmr_uvi *uvi)
{
	strbuf_reset(&logger->data);
	strbuf_printf(
		&logger->data,
		"%u",
		uvi->index);

	update(logger, logger->cfg.uvi_rrd);
}

static void log_baro(struct rrd_logger *logger, struct wmr_baro *baro)
{
	strbuf_reset(&logger->data);
	strbuf_printf(
		&logger->data,
		"%u:%u",
		baro->pressure,
		baro->alt_pressure);

	update(logger, logger->cfg.baro_rrd);
}

static void log_temp(struct rrd_logger *logger, struct wmr_temp *temp)
{
	struct strbuf filename; /* filename depends on sensor ID */

	strbuf_reset(&logger->data);
	strbuf_printf(
		&logger->data,
		"%.1f:%u:%.1f",
		temp->temp,
		temp->humidity,
		temp->dew_point);

	strbuf_init(&filename, 128);
	strbuf_printf(&filename, logger->cfg.temp_N_rrd, temp->sensor_id);
	update(logger, strbuf_get_string(&filename));
	strbuf_free(&filename);
}

static void log_reading(struct rrd_logger *logger, struct wmr_reading *reading)
{
	switch (reading->type) {
	case WMR_WIND:
		log_wind(logger, &reading->wind);
		break;
	case WMR_RAIN:
		log_rain(logger, &reading->rain);
		break;
	case WMR_UVI:
		log_uvi(logger, &reading->uvi);
		break;
	case WMR_BARO:
		log_baro(logger, &reading->baro);
		break;
	case WMR_TEMP:
		log_temp(logger, &reading->temp);
		break;
	}
}

void rrd_log_reading(struct wmr200 *wmr, struct wmr_reading *reading, void *arg)
{
	(void) wmr;
	struct rrd_logger *logger = (struct rrd_logger *)arg;
	log_reading(logger, reading);
}

void rrd_logger_init(struct rrd_logger *logger)
{
	strbuf_init(&logger->data, 128);
}

void rrd_logger_free(struct rrd_logger *logger)
{
	strbuf_free(&logger->data);
}
