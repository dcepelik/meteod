# Oregon Scientific WMR200 weather station tools


## Abstract

This project consists of several Unix tools which are of interest to some owners
of the Oregon Scientific WMR200 weather station.

* communication wrapper which speaks the proprietary protocol of WMR200 and
	wraps weather station's readings into well-defined data structures
* `wmrd`, Unix daemon talking to the WMR200 and logging all readings to one or
	several of the available logging back-ends
* `wmrc`, client to the server component of `wmrd` capable of querying current
	readings over TCP/IP
* `wmrformat`, Perl script which queries current readings using `wmrc` and allows
	you to print nice formatted strings

A complementary project [wmr200-website](https://github.com/dcepelik/wmr200-website.git)
exists which provides implementation of a simple website using forementioned
tools to present your station's readings on the web with pretty much everything
you would expect.


## Installation

Run `make` to make the application and `make install` to install the binaries
to `/usr/bin`.

If you plan on using the application's RRD logger, create RRD files from scratch
using `rrd_create.sh` script bundled with the sources. For example, to create
RRD files at `/var/wmrd/rrd`, one would execute following commands:

	mkdir -p /var/wmrd/rrd
	./rrd_create /var/wmrd/rrd


### Dependencies

* HIDAPI (`hidapi-libusb`)
* `librrd`


## Usage

The following assumes that you have installed `wmrd` onto your "server" (by
"server" we mean the machine that's connected to your WMR200).

### Logging to files and RRD databases

### Transferring data over TCP/IP

### Website integration
