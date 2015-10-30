#ifndef MACROS_H
#define MACROS_H


#define WMR200_VID	0x0FDE
#define WMR200_PID	0xCA01

#define FRAME_SIZE		8	// Bytes
#define HEARTBEAT_INTERVAL	30	// seconds
#define MAX_EXT_SENSORS		10

#define HEARTBEAT		0xD0
#define HISTORIC_DATA_NOTIF	0xD1
#define HISTORIC_DATA		0xD2
#define WIND_DATA		0xD3
#define RAIN_DATA		0xD4
#define UVI_DATA		0xD5
#define BARO_DATA		0xD6
#define TEMP_HUMID_DATA		0xD7
#define STATUS_DATA		0xD9
#define REQUEST_HISTORIC_DATA	0xDA
#define LOGGER_DATA_ERASE	0xDB
#define COMMUNICATION_STOP	0xDF

#define SIGN_POSITIVE		0x0
#define SIGN_NEGATIVE		0x8

#define NTH_BIT(n, val)		(((val) >> (n)) & 1)
#define HIGH(b)			LOW((b) >> 4)
#define LOW(b)			((b) &  0xF)


#endif
