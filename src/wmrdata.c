/*
 * wmrdata.c:
 * WMR data structures (encapsulation of readings) and utils
 *
 * This software may be freely used and distributed according to the terms
 * of the GNU GPL version 2 or 3. See LICENSE for more information.
 *
 * Copyright (c) 2015 David Čepelík <cepelik@gymlit.cz>
 */

#include "wmrdata.h"
#include "strbuf.h"


char *
wmr_sensor_name(wmr_reading *reading)
{
	strbuf buf;

	switch (reading->type) {
	case WMR_WIND:
		return "wind";

	case WMR_RAIN:
		return "rain";

	case WMR_UVI:
		return "uvi";

	case WMR_BARO:
		return "baro";

	case WMR_TEMP:
		strbuf_init(&buf);
		strbuf_append(&buf, "temp%u", reading->temp.sensor_id);
		char *out = strbuf_copy(&buf);
		strbuf_free(&buf);

		return out;

	case WMR_STATUS:
		return "status";

	case WMR_META:
		return "meta";
	}

	return NULL;
}

const char *packet_type_to_string(enum packet_type type)
{
	switch (type) {
	case PACKET_ERASE_ACK:
		return "PACKET_ERASE_ACK";
	case PACKET_HISTDATA_NOTIF:
		return "PACKET_HISTDATA_NOTIF";
	case PACKET_STOP_ACK:
		return "PACKET_STOP_ACK";
	case HISTORIC_DATA:
		return "HISTORIC_DATA";
	case WMR_WIND:
		return "WMR_WIND";
	case WMR_RAIN:
		return "WMR_RAIN";
	case WMR_UVI:
		return "WMR_UVI";
	case WMR_BARO:
		return "WMR_BARO";
	case WMR_TEMP:
		return "WMR_TEMP";
	case WMR_STATUS:
		return "WMR_STATUS";
	case WMR_META:
		return "WMR_META";
	case PACKET_TYPE_MAX:
		break;
	}

	return NULL;
}
