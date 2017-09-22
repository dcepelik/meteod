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
	you to print nicely formatted strings

A complementary project [wmr200-website](https://github.com/dcepelik/wmr200-website.git)
exists which provides implementation of a simple website using forementioned
tools to present your station's readings on the web with pretty much everything
you would expect.

You can see everything in action at [our WMR200 website](http://meteo.gymlit.cz).


## Installation

Run `make` to make the application and `make install` to install the binaries
to `/usr/bin`

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

## Implementation

### The big picture

In short, the daemon reads byte-by-byte from the USB communication channel.
It keeps reading until the type of packet, it's length and all the payload
is read in memory. Then, the packet is handed over to a dispatch routine which,
depending on the type of packet, calls a `process_` routine which interpretes
the payload and wraps the data into structures such as `wmr_wind`, `wmr_rain`
etc.

Once processed, the data is then passed to one or more handlers (functions),
depending on your precise setup. These handlers provide the actual functionality
needed to do anything useful with the data:

* The `rrd` logger and it's handler function `log_to_rrd` take a reading and
  save the data into an RRD database file.

* The `server` logger (`log_to_server`) keeps the last reading of every kind
  (rain, wind, etc.) and makes it available on the network. The communication
  protocol is trivial -- once you connect to the server, you'll get all the
  most recent readings.

  (There's a client implementation called `wmrformat`. It's a simple Perl script.)

* The `yaml` (`log_to_yaml`) just serializes the readings into YAML format. So
  you can, for example, store them on disk.

Two threads participate in these action, the `mainloop` thread doing the
forementioned byte-by-byte reading, and the `heartbeat` thread which sends the
device a heartbeat packet every 30 seconds to keep the communication alive.
(More precisely, to keep the station sending data over the wire "in real time"
instead of keeping it in internal memory (the data logger).

(Actually, more threads come into play when you use the server component. It has
some threads of it's own.)

If the daemon wasn't running for some time, the station probably started logging
data internally. When it's not busy doing other stuff, it will send a
`HISTORIC_DATA_NOTIF` packet telling us there are some unprocessed (meaning not
given to us) historic records. We react by sending a `REQUEST_HISTORIC_DATA` packet.
Then historic data packets (type `0xD7`) are sent. (I think this usually happens once
per the heartbeat period, but I'm not sure.)

If you just want to get rid of stala data because you don't care about it at all
(because for example you only use the station as a "real-time" monitor), you may
send a `LOGGER_DATA_ERASE` packet. Done.
