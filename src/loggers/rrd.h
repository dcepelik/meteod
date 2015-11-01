/*
 * loggers/rrd.h:
 * Log WMR readings to RRD files
 *
 * This software may be freely used and distributed according to the terms
 * of the GNU GPL version 2 or 3. See LICENSE for more information.
 *
 * Copyright (c) 2015 David Čepelík <cepelik@gymlit.cz>
 */


#ifndef LOGGERS_RRD_H
#define LOGGERS_RRD_H

#include "wmr200.h"


#define WIND_RRD	"wind.rrd"
#define RAIN_RRD	"rain.rrd"
#define UVI_RRD		"uvi.rrd"
#define BARO_RRD	"baro.rrd"
#define TEMPN_RRD	"temp%i.rrd"


void log_to_rrd(wmr_reading *reading, char *rrd_file);


#endif
