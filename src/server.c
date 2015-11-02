/*
 * server.c:
 *
 *
 * This software may be freely used and distributed according to the terms
 * of the GNU GPL version 2 or 3. See LICENSE for more information.
 *
 * Copyright (c) 2015 David Čepelík <cepelik@gymlit.cz>
 */


#include "server.h"


struct latest_data {
	wmr_wind wind;
	wmr_rain rain;
	wmr_uvi uvi;
	wmr_baro baro;
	wmr_temp temp;
	wmr_status status;
};
