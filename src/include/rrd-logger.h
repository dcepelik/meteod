#ifndef RRD_LOGGER_H
#define	RRD_LOGGER_H

#include "wmr200.h"
#include "strbuf.h"

/*
 * RRD logger configuration.
 */
struct rrd_cfg
{
	char *rrd_root;		/* RRD files root directory */
	char *wind_rrd;		/* wind database */
	char *rain_rrd;		/* rain database */
	char *uvi_rrd;		/* UV index database */
	char *baro_rrd;		/* barometric database */
	char *temp_N_rrd;	/* temperature database of Nth sensor */
};

/*
 * Execution context of an RRD logger.
 */
struct rrd_logger
{
	struct rrd_cfg cfg;
	struct strbuf data;
};

void rrd_logger_init(struct rrd_logger *logger);
void rrd_logger_free(struct rrd_logger *logger);

void rrd_log_reading(struct wmr200 *wmr, struct wmr_reading *reading, void *arg);

#endif
