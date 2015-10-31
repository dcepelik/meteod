#!/bin/bash

#rrdtool create rain.rrd \
#	--step 600
#	DS:rate:GAUGE:1200:U:U \
#	DS:total:COUNTER:1200:U:U


#rrdtool create wind.rrd \
#	--step 600 \
#	DS:speed:GAUGE:1200:U:U \
#	DS:gust_speed:GAUGE:1200:U:U


rrdtool create temp.rrd \
	--step 5 \
	DS:temp_0:GAUGE:600:U:U \
	DS:temp_1:GAUGE:600:U:U \
	RRA:AVERAGE:0.5:1:20


#rrdtool create pressure.rrd \
#	--step 300
#	DS:pressure:GAUGE:600:U:U \
#	DS:alt_pressure:GAUGE:600:U:U



