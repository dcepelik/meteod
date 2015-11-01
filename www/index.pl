#!/usr/bin/perl

use strict;
use warnings;

use RRDs;


sub create_graph {
	my ($file, $start, $title, @graph_def) = @_;

	RRDs::graph(
		$file,
		'--start', $start,
		'--title', $title,
		#'--watermark', 'meteo.gymlit.cz <meteo@gymlit.cz>',
		#'--legend-position=west',
		'--slope-mode',
		'-w', 500,
		@graph_def,
	);

	print RRDs::error, "\n";
}


sub get_temp_graph_def {
	my $id = shift;

	return (
		'--vertical-label', 'st C',
		"DEF:temp_avg=/var/wmrd/rrd/temp$id.rrd:temp:AVERAGE",
		"DEF:temp_max=/var/wmrd/rrd/temp$id.rrd:temp:MAX",
		"DEF:temp_min=/var/wmrd/rrd/temp$id.rrd:temp:MIN",
		"DEF:dewpoint=/var/wmrd/rrd/temp$id.rrd:dewpoint:AVERAGE",
		'LINE1:temp_max#FF0000:maximální teplota',
		'LINE1:temp_min#0000FF:minimální teplota',
		'LINE1:temp_avg#000000:průměrná teplota',
		'LINE1:dewpoint#00FF00:rosný bod',
	);
}


my @humidity_graph_def = (
	'--vertical-label', 'procenta',
	'DEF:humidity=/var/wmrd/rrd/temp0.rrd:humidity:AVERAGE',
	'LINE1:humidity#000000',
);


my @wind_graph_def = (
	'--vertical-label', 'm/s',
	'DEF:gust_speed=/var/wmrd/rrd/wind.rrd:gust_speed:MAX',
	'DEF:avg_speed=/var/wmrd/rrd/wind.rrd:avg_speed:AVERAGE',
	'LINE1:avg_speed#000000:průměrná rychlost',
	'LINE1:gust_speed#FF0000:rychlost poryvů',
);

my @pressure_graph_def = (
	'--vertical-label', 'hPa',
	'DEF:pressure=/var/wmrd/rrd/baro.rrd:pressure:AVERAGE',
	'LINE1:pressure#000000',

);


my @rain_graph_def = (
	'--vertical-label', 'mm/h',
	'DEF:rate=/var/wmrd/rrd/rain.rrd:rate:AVERAGE',
	'LINE1:rate#000000:průměrné srážky',
);

create_graph("img/temp0-day.png", "-8h", "Teplota v serverovně", get_temp_graph_def(0));
create_graph("img/temp1-day.png", "-8h", "Venkovní teplota 9 metrů nad povrchem", get_temp_graph_def(1));
create_graph("img/humidity-day.png", "-8h", "Venkovní relativní vzdušná vlhkost", @humidity_graph_def);
create_graph("img/humidity-day.png", "-8h", "Relativní vzdušná vlhkost", @humidity_graph_def);
create_graph("img/wind-day.png", "-12h", "Rychlost větru", @wind_graph_def);
create_graph("img/pressure-day.png", "-8h", "Atmosférický tlak", @pressure_graph_def);
create_graph("img/rain-day.png", "-8h", "Srážky", @rain_graph_def);
