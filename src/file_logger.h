#ifndef STDOUT_H
#define STDOUT_H

#include <stdio.h>

#include "wmr200.h"


void log_to_file(struct wmr_reading *reading, FILE *stream);


#endif
