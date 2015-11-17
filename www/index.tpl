<!DOCTYPE html>
<html>
	<head>
		<title>meteo.gymlit.cz</title>
		<meta charset="utf-8" />

		<link rel="stylesheet" href="css/screen.css" media="all">

		<script src="https://ajax.googleapis.com/ajax/libs/jquery/2.1.4/jquery.min.js"></script>
		<script type="text/javascript">
			function reloadGraphs() {
				var date = new Date();

				$("img").each(function(el) {
					var $el = $(this);
					var src = $el.attr('src').replace(/\?timestamp=.*/, '');
					console.log(src);
					$el.attr('src', $el.attr('src') + '?timestamp=' + date.getTime())
				});

				setTimeout(reloadGraphs, 5000);
			}

			setTimeout(reloadGraphs, 5000);
		</script>
	</head>

	<body>
		<div id="page">
			<h1>meteo.gymlit.cz</h1>

			<div id="sidebar">
				<h3>Aktuální údaje</h3>
				<table class=lr>
					<tr>
						<th rowspan=2>Teplota</th>
						<td>Skutečná</td>
						<td class=fresh>22.4 &deg;C</td>
					</tr>
					<tr>
						<td>Pocitová</td>
						<td>14.4 &deg;C</td>
					</tr>

					<tr>
						<th colspan=2>Rosný bod</th>
						<td>9 &deg;C</td>
					</tr>

					<tr>
						<th colspan=2>Vzdušná vlhkost</th>
						<td class=fresh>64 %</td>
					</tr>

					<tr>
						<th colspan=2>Srážky</th>
						<td>0.8 mm&middot;hod<sup>-1</sup></td>
					</tr>

					<tr>
						<th colspan=2>Tlak</th>
						<td>1035 hPa</td>
					</tr>

					<tr>
						<th colspan=2>Předpověď 12-24 h</th>
						<td>V noci zataženo</td>
					</tr>

					<tr>
						<th rowspan=2>Rychlost větru</th>
						<td>Poryvy</td>
						<td>2 m&middot;s<sup>-1</sup></td>
					</tr>

					<tr>
						<td>Průměrná</td>
						<td>0.8 m&middot;s<sup>-1</sup></td>
					</tr>

					<tr>
						<th colspan=2>Směr větru</th>
						<td>SSV</td>
					</tr>

					<tr>
						<th colspan=2>UV index</th>
						<td>7/15</td>
					</tr>
				</table>

				<h3>O stanici</h3>
				<img src="img/meteo.jpg" alt="Pohled na meteostanici" class="fright" />

				<p>
					Naše meteostanice je ta nejkrásnější
					na Proseku, protože je tu jediná.
				</p>

				<p>
					Více informací o ní naleznete na
					<a href="#">wiki.gymlit.cz</a>.
				</p>

				<h3>Senzor konzole</h3>
				<table class=lr>
					<tr>
						<th colspan=2>Teplota</th>
						<td class=fresh>18.2 &deg;C</td>
					</tr>
					
					<tr>
						<th colspan=2>Rosný bod</th>
						<td>8 &deg;C</td>
					</tr>

					<tr>
						<th colspan=2>Vzdušná vlhkost</th>
						<td class>44 %</td>
					</tr>
				</table>


				<h3>Statistika</h3>
				<table class=lr>
					<tr>
						<th>Paketová rychlost</th>
						<td>136 p&middot;s<sup>-1</sup></td>
					</tr>

					<tr>
						<th>Chybovost</th>
						<td>0.17 %</td>
					</tr>

					<tr>
						<th>Průměrné zpoždění dat</th>
						<td>11 s</td>
					</tr>

					<tr>
						<th>Stáří nejnovějšího paketu</th>
						<td class=fresh>3 s</td>
					</tr>

					<tr>
						<th>Drift systémových hodin</th>
						<td>+ 7 s</td>
					</tr>
				</table>

				<h3>Komponenty</h3>
				<table class=tc>
					<tr>
						<th>Senzor</th>
						<th>Baterie</th>
						<th>Signál</th>
					</tr>

					<tr>
						<td>Anemometr</td>
						<td><img src="/img/accept.png" alt="OK" /></td>
						<td><img src="/img/accept.png" alt="OK" /></td>
					</tr>

					<tr>
						<td>Srážkoměr</td>
						<td><img src="/img/accept.png" alt="OK" /></td>
						<td><img src="/img/accept.png" alt="OK" /></td>
					</tr>

					<tr>
						<td>UV senzor</td>
						<td><img src="/img/accept.png" alt="OK" /></td>
						<td><img src="/img/accept.png" alt="OK" /></td>
					</tr>

					<tr>
						<td>Teplotní čidlo č. 1</td>
						<td class=error>Slabá</td>
						<td><img src="/img/accept.png" alt="OK" /></td>
					</tr>

					<tr>
						<td>RTC obvod</td>
						<td>&mdash;</td>
						<td><img src="/img/accept.png" alt="OK" /></td>
					</tr>
				</table>
			</div>

			<div id="main">
				<p>
					<strong>meteo.gymlit.cz</strong> je web naší školní
					meteostanice. Na této stránce naleznete informace
					o aktuálních měřeních a průběhy všech podstatných
					parametrů počasí v nedávné době.
				</p>


				<p class=warning>
					<strong>Pozor prosím,</strong> zde zobrazené jsou pouze
					modelová data, která <strong>neodpovídají aktuální povětrnostní
					situaci</strong>!
				</p>

				<h2>Data zapisovače</h2>

				<h3>Teplota</h3>
				<p>
					Teplota je měřena cca 10 m nad zemí.
				</p>
				<img src="graphs/temp1/12h.png" alt="" />

				<h3>Tlak</h3>
				<img src="graphs/pressure/12h.png" alt="" />

				<h3>Rychlost větru</h3>
				<img src="graphs/wind/12h.png" alt="" />

				<h3>Vlhkost</h3>
				<img src="graphs/humidity/12h.png" alt="" />

				<h3>Srážky</h3>
				<img src="graphs/rain/12h.png" alt="" />
			</div>

			<div class="cb"></div>
		</div>
	</body>
</html>
