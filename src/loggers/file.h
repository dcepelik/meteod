/*
 * loggers/file.h:
 * Log WMR readings to a file
 *
 * This software may be freely used and distributed according to the terms
 * of the GNU GPL version 2 or 3. See LICENSE for more information.
 *
 * Copyright (c) 2015 David Čepelík <cepelik@gymlit.cz>
 */


#ifndef LOGGERS_FILE_H
#define	LOGGERS_FILE_H

#include "wmr200.h"
#include <stdio.h>


void file_push_reading(wmr_reading *reading, void *arg);


#endif
