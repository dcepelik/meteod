# Oregon Scientific WMR200 weather station tools


## Abstract

This project consists of several Unix tools which are of interest to WMR200
weather station owners:

* USB HID communication wrapper which speaks the proprietary protocol of WMR200
	and wraps weather station's readings into well-defined data structures
* `wmrd`, Unix daemon talking to the WMR200 and logging all readings to one of
	the logging back-ends
* `wmrc`, client to the server component of `wmrd` capable of querying current
	readings over TCP/IP

A complementary project, [wmr200_website](https://github.com/dcepelik/wmr200_website.git),
exists which provides implementation of a simple website using forementioned
tools to present your station's readings on the web.


## Installation and usage

Run `make` to make the application and `make install` to install the binaries
to `/usr/bin`.

If you plan on using the application's RRD logger, create RRD files from scratch
using `rrd_create.sh` script bundled with the sources. For example, to create
RRD files at `/var/wmrd/rrd`, one would execute following commands:

	mkdir -p /var/wmrd/rrd
	./rrd_create /var/wmrd/rrd

Usage instructions as well as other useful information may be found in the
projects [wiki](https://github.com/dcepelik/wmr200_tools/wiki).

### Dependencies

* HIDAPI (`hidapi-libusb`)
* `librrd`
