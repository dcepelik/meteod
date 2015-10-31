#!/bin/bash

rrdtool create rain.rrd \
	--step 600 \
	DS:rate:GAUGE:1200:0:600 \
	DS:total:COUNTER:1200:U:U \
	RRA:AVERAGE:0.5:1:144 \
	RRA:AVERAGE:0.5:12:144 \
	RRA:AVERAGE:0.5:72:144 \
	RRA:MAX:0.5:1:144 \
	RRA:MAX:0.5:12:144 \
	RRA:MAX:0.5:72:144


rrdtool create wind.rrd \
	--step 180 \
	DS:avg_speed:GAUGE:360:0:100 \
	DS:gust_speed:GAUGE:360:0:100 \
	RRA:AVERAGE:0.5:1:480 \
	RRA:AVERAGE:0.5:10:480 \
	RRA:AVERAGE:0.5:60:480 \
	RRA:MAX:0.5:1:480 \
	RRA:MAX:0.5:10:480 \
	RRA:MAX:0.5:60:480


rrdtool create baro.rrd \
	--step 180 \
	DS:pressure:GAUGE:360:800:1200 \
	DS:alt_pressure:GAUGE:360:800:1200 \
	RRA:AVERAGE:0.5:1:480 \
	RRA:AVERAGE:0.5:10:480 \
	RRA:AVERAGE:0.5:60:480 \
	RRA:MAX:0.5:1:480 \
	RRA:MAX:0.5:10:480 \
	RRA:MAX:0.5:60:480


rrdtool create uvi.rrd \
	--step 180 \
	DS:rate:GAUGE:1200:0:600 \
	DS:total:COUNTER:1200:U:U \
	RRA:AVERAGE:0.5:20:24 \
	RRA:AVERAGE:0.5:80:24 \
	RRA:MAX:0.5:20:24 \
	RRA:MAX:0.5:80:24


for i in $(seq 0 10); do
	rrdtool create temp$i.rrd \
		--step 180 \
		DS:temp:GAUGE:360:0:100 \
		DS:humidity:GAUGE:360:0:100 \
		DS:dewpoint:GAUGE:360:0:100 \
		RRA:AVERAGE:0.5:1:480 \
		RRA:AVERAGE:0.5:10:480 \
		RRA:AVERAGE:0.5:60:480 \
		RRA:MAX:0.5:1:480 \
		RRA:MAX:0.5:10:480 \
		RRA:MAX:0.5:60:480 \
		RRA:MIN:0.5:1:480 \
		RRA:MIN:0.5:10:480 \
		RRA:MIN:0.5:60:480

done
