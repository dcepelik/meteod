#ifndef CONFIG_H
#define CONFIG_H

#include "rrd-logger.h"
#include "server.h"
#include <sys/types.h>

/*
 * Program configuration.
 *
 * TODO: In the future, add support for run-time configuration
 *       loaded from files, shall it ever be needed.
 */
struct
{
	struct rrd_cfg rrd;		/* RRD logger configuration */
	struct wmr_server_cfg srv;	/* WMR server configuration */
	unsigned reconnect_default;	/* default reconnection interval */
	unsigned reconnect_max;		/* maximum reconnection interval */
	mode_t umask;			/* umask to be set */
	char *user;			/* setuid user name */
	char *group;			/* setgid user name */
	char *chdir;			/* directory to chroot to */
	uid_t uid;			/* uid obtained from user name */
	gid_t gid;			/* gid obtained from group name */
} cfg = {
	.rrd = {
		.rrd_root = "/var/meteod",
		.wind_rrd = "wind.rrd",
		.rain_rrd = "rain.rrd",
		.uvi_rrd = "uvi.rrd",
		.baro_rrd = "baro.rrd",
		.temp_N_rrd = "temp%i.rrd",
	},
	.srv = {
		.port = 20892,
	},
	.reconnect_default = 1,
	.reconnect_max = 300,
	.umask = 0227,
	.user = "meteod",
	.group = "meteod",
	.chdir = "/var/meteod"
};

#endif
