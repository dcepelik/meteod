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
