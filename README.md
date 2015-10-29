# WRM200 datalogger

## Overview

This is a very, very simple data logger for the Oregon Scienitif WMR200 weather
stations.

When connected to a PC, WMR200 is capable of a simple dialogue during which it
transfers current (and possibly also past) weather data. The protocol itself is
not open, but has been fairly well understood by enthusiasts.

This datalogger can talk and listen to the WMR200 and understand almost all of
the readings you can see on the console itself:

* Wind direction, wind gust speed, wind average speed and wind chill
* Rain rate, rain during last hour, rain in last 24 hours, rain total
* Pressure and altitude pressure, basic "weather forecast" (cloudy, sunny, rainy, ...)
* Temperature, humidity, dewpoint and heat index
* System operational status: batteries, signal levels, etc.


## Project status

This program is under active development and cannot be considered stable at the
time of writing, because severe modifications will follow soon.

But if you are a C developer (or if you have a lot of enthusiasm), it may be
of use or interest to you anyway.

Just don't expect things won't change. I haven't tested the outputs thorougly,
so be careful.

If you find any errors, **please do let me know!**


### Plan

* Write a Perl processor of `logger`'s output and push all data to RRDtool
* Write a CGI Perl script to host a nice little website presenting current
	and past readings with nice graphs from RRDtool
* Thorougly test everything
* Daemonize the logger and the Perl script

## Dependencies

HIDAPI library is required.


## Installation and usage

Just `make` it and run `logger`. Output is (at the moment) only possible to
`stdout`.


### Output format

Obvious when you look at the output:

	# packet 0xD2 (42 bytes)
	1448835240 {
		rain.rate: 0.00
		rain.hour: 0.00
		rain.24h: 6629.40
		rain.accum: 14782.80
		wind.dir: WSW
		wind.gust_speed: 0.00
		wind.avg_speed: 0.00
		wind.chill: -17.8
		uvi.index: 15
		baro.pressure: 994
		baro.alt_pressure: 1028
		baro.forecast: clear
		indoor.humidity: 44
		indoor.heat_index: 0
		indoor.temp: 22.1
		indoor.dew_point: 9.0
	}

The `#` starts a comment, the numeric value `1448835240` is time of the readings
(seconds since the epoch).


## More info

* This tool was inspired by [Barnybug's WRM100 datalogger](https://github.com/barnybug/wmr100)
* [WMR200 protocol](http://www.bashewa.com/wmr200-protocol.php)
* [HIDAPI website](http://www.signal11.us/oss/hidapi/)
