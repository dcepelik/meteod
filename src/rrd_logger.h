#ifndef RRD_LOGGER_H
#define RRD_LOGGER_H

#include "wmr200.h"


void log_to_rrd(struct wmr_reading *reading, char *rrd_file);


#endif
