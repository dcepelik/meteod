#ifndef LOGGERS_FILE_H
#define LOGGERS_FILE_H

#include "wmr200.h"
#include <stdio.h>


void log_to_file(struct wmr_reading *reading, FILE *stream);


#endif
