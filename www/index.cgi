#!/usr/bin/perl

use strict;
use warnings;

use RRDs;


my %graph_def = (
	humidity => [
		'--vertical-label', 'procenta',
		'DEF:humidity=/var/wmrd/rrd/temp0.rrd:humidity:AVERAGE',
		'LINE1:humidity#000000',
	],

	wind => [
		'--vertical-label', 'm/s',
		'DEF:gust_speed=/var/wmrd/rrd/wind.rrd:gust_speed:MAX',
		'DEF:avg_speed=/var/wmrd/rrd/wind.rrd:avg_speed:AVERAGE',
		'AREA:gust_speed#FF0000:rychlost poryvů',
		'LINE1:avg_speed#000000:průměrná rychlost',
	],

	pressure => [
		'--vertical-label', 'hPa',
		'DEF:pressure=/var/wmrd/rrd/baro.rrd:pressure:AVERAGE',
		'LINE1:pressure#000000',
	],

	rain => [
		'--vertical-label', 'mm/h',
		'DEF:rate=/var/wmrd/rrd/rain.rrd:rate:AVERAGE',
		'LINE1:rate#000000:průměrné srážky',
	],
);

# in this setup, all temperature graphs are defined the same
for my $i (0..10) {
	$graph_def{"temp$i"} = [
		'--vertical-label', 'st C',
		"DEF:temp_avg=/var/wmrd/rrd/temp$i.rrd:temp:AVERAGE",
		"DEF:temp_max=/var/wmrd/rrd/temp$i.rrd:temp:MAX",
		"DEF:temp_min=/var/wmrd/rrd/temp$i.rrd:temp:MIN",
		"DEF:dewpoint=/var/wmrd/rrd/temp$i.rrd:dewpoint:AVERAGE",
		'LINE1:temp_max#FF0000:maximální teplota',
		'LINE1:temp_min#0000FF:minimální teplota',
		'LINE1:temp_avg#000000:průměrná teplota',
		'LINE1:dewpoint#00FF00:rosný bod',
	];
}


sub create_graph {
	my ($file, $start, @graph_def) = @_;

	RRDs::graph(
		$file,
		'--start', $start,
		'--slope-mode',
		'-w', 500,
		@graph_def,
	);
}





my $uri = $ENV{REQUEST_URI};

if ($uri =~ m{^/?graphs/(wind|rain|pressure|humidity|temp[0-9]+)/([0-9]+\w).png}) {
	my ($sensor, $delay) = ($1, $2);

	my $defref = $graph_def{$sensor};
	my $filename = "temp/$sensor.png";

	unlink "temp/img.png";
	create_graph($filename, "-$2", @$defref);

	open my $png, "<", $filename;

	print "Content-type: image/png\n";
	print "Content-length: ", (stat($png))[7], "\n\n";
	
	my $data;
	print $data while read($png, $data, 16384) > 0;
}
elsif ($uri =~ m{^/?(index.cgi)?$}) {
	print "Content-type: text/html\n\n";

	my $string;
	{
		local $/ = undef;
		open my $file, "index.tpl" or die "Couldn't open file: $!";
		binmode $file;
		$string = <$file>;
		close $file;
	}

	print $string;
}
else {
	print <<"RESP";
Content-type: text/html
Status: 404 Not Found

<!DOCTYPE html>
<html>
	<head>
		<title>meteo.gymlit.cz</title>
		<meta charset="utf-8" />
		
		<link rel="stylesheet" href="css/screen.css" media="all" />
	</head>

	<body>
		<div id="page">
			<h1>meteo.gymlit.cz</h1>
			<p class=warning>
				<strong>Neplatný požadavek.</strong>
				Sorry, Johnny. <a href="/">Zpět na hlavní stránku &rarr;</a>
			</p>
		</div>
	</body>
</html>
RESP
}
