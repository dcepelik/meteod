/*
 * wmr200.h:
 * Oregon Scientific WMR200 USB HID communication wrapper
 *
 * This software may be freely used and distributed according to the terms
 * of the GNU GPL version 2 or 3. See LICENSE for more information.
 *
 * Copyright (c) 2015 David Čepelík <cepelik@gymlit.cz>
 */

#ifndef WMR200_H
#define	WMR200_H

#include "wmrdata.h"
#include "common.h"

#include <stdio.h>
#include <hidapi.h>
#include <pthread.h>


#define	WMR200_MAX_TEMP_SENSORS		10

struct wmr200;


typedef void wmr_logger_t(struct wmr200 *wmr, struct wmr_reading *reading, void *arg);
typedef void wmr_err_handler_t(struct wmr200 *wmr, void *arg);

/*
 * Initialize the WMR200 module.
 */
void wmr_init();

/*
 * Dispose global resources hold by WMR200 module.
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
