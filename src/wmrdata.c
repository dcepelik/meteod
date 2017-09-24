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
		switch (reading->temp.sensor_id) {
		case 0:
			return "console";
		case 1:
			return "ext1";
		case 2:
			return "ext2";
		case 3:
			return "ext3";
		case 4:
			return "ext4";
		case 5:
			return "ext5";
		case 6:
			return "ext6";
		case 7:
			return "ext7";
		case 8:
			return "ext8";
		case 9:
			return "ext9";
		case 10:
			return "ext10";
		default:
			return NULL;
		}

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
