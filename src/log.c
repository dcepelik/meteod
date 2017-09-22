/*
 * log.c:
 * Logging and debugging functions
 *
 * This software may be freely used and distributed according to the terms
 * of the GNU GPL version 2 or 3. See LICENSE for more information.
 *
 * Copyright (c) 2015 David Čepelík <cepelik@gymlit.cz>
 */


#include <stdarg.h>
#include <stdlib.h>
#include <syslog.h>


static void
log_vmsg(int priority, char *format, va_list ap)
{
	vsyslog(priority, format, ap); 
}


/*
 * public interface
 */


void
log_open_syslog(void)
{
	openlog(NULL, LOG_NOWAIT | LOG_PID, LOG_USER);
}


void
log_msg(int priority, char *format, ...)
{
	va_list ap;
	va_start(ap, format);

	log_vmsg(priority, format, ap);

	va_end(ap);
}


void
log_error(char *format, ...)
{
	va_list ap;
	va_start(ap, format);

	log_vmsg(LOG_ERR, format, ap);

	va_end(ap);
}


void
log_warning(char *format, ...)
{
	va_list ap;
	va_start(ap, format);

	log_vmsg(LOG_WARNING, format, ap);

	va_end(ap);
}


void
log_info(char *format, ...)
{
	va_list ap;
	va_start(ap, format);

	log_vmsg(LOG_INFO, format, ap);

	va_end(ap);
}


void
log_debug(char *format, ...)
{
	va_list ap;
	va_start(ap, format);

	log_vmsg(LOG_DEBUG, format, ap);

	va_end(ap);
}


void
log_exit(char *format, ...)
{
	va_list ap;
	va_start(ap, format);

	log_vmsg(LOG_ALERT, format, ap);

	va_end(ap);
	exit(EXIT_FAILURE);
}
