# Oregon Scientific WRM200 weather station toolbox

## Overview

This project consists of several tools that are of interest to WMR200 owners:

* `wmr200`, USB HID communication wrapper that speaks the proprietary protocol
	of WMR200 and encapsulates readings into well-defined data structures.
* `wmrd`, data logger for the weather station.
* `wmrc`, a client to `wmrd` server component that can query current readings
over TCP/IP for you.

These tools provide you with just enough power to build a website showing your
weather stations current and past readings with nice `rrdtool` graphs and your
station's operational status. An example website is included to get you started.

###  `wmr200`

WMR200 allows you to connect to your station's console over USB and read it's
current (and possibly past) readings, which include:

* **Wind readings**---direction, gust speed, average speed and wind chill
* **Rain readings**---rain rate, total for the last hour, total for 24 hours and
	total since always
* **UV index**
* **Barometric readings**---pressure, pressure at sea level and a simple
	"weather forecast"
* **Temperature/humidity readings**---temperature, humidity, dewpoint
	and heat index
* **Operational status**---batteries and signal levels for individual sensors,
	RTC signal level

### `wmrd`

This is the data logger itself. Several (two) "loggers" are provided:
`file_logger` and `rrd_logger`.

File logger logs readings into a file in a straight-forward format, `rrd_logger`
saves readings into round-robin database (RRD) files.

The loggers may be registered as "data handlers" with `wmr200` wrapper. You can
have as many handlers as you wish, including your own.

## Project status

At this moment, everything is work-in-progress. If you find any errors,
**please do let me know!**

## Dependencies

* `libhidapi`
* `librrd`

## Installation and usage

Just `make` it. The rest needs to be figured by you right now. A good starting
place would be too run `wmrd` and have a look at `wmrd.c` to see what it does
and how.

If you plan on using `rrd_logger`, you may want to (customize and) run
`rrd_create.sh` first to kick-start a simple database.
