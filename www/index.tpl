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
			<p>
				<strong>meteo.gymlit.cz</strong> je web naší školní
				meteostanice. Na této stránce naleznete informace
				o aktuálních měřeních a průběhy všech podstatných
				parametrů počasí v nedávné době.
			</p>

			<p class=warning>
				<strong>Pozor prosím,</strong> zde zobrazené
				údaje a celý tento web jsou betaverzí softwarového
				díla, které je doposud ve výstavbě.
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
	</body>
</html>
