<?php echo '<?xml version="1.0" encoding="UTF-8"?>' ?>
<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Strict//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd">
<html xmlns="http://www.w3.org/1999/xhtml" xml:lang="en" lang="en">
<head>
  <title>Gaming and other sins</title>
  <meta http-equiv="content-type" content="text/html;charset=utf-8" />
  <link rel="stylesheet" type="text/css" href="stylesheets/style.css" />  
</head>

<body>
<div class="mySite">
<div class="oben"></div>
<div class="menue">
<ul>
<li class="menue"><a href="index.php">Home</a></li>
<li class="menue"><a href="games.php">Games</a></li>
<li class="menue"><a href="tos.php">Terms of Service</a></li>
<li class="menue"><br /></li>
<li class="menue"><a href="impressum.php">Impressum</a></li>
</ul>
</div>
<div class="rahmenMain">
<div class="anmeldung">
<div class="anmeldemain"></div>
<div class="anmeldebox">

<form action="login.php" method="post">
<p class="anmeld"> Email-Adresse:&nbsp; <input type="text" name="user" value="" ></input> Passwort:&nbsp; <input type="password" name="password" value="" ></input>
<input type="submit" value="Login"/></p>
</form>

<form action="registration.php" method="post"><p>
<input type="submit" value="Registration"/></p>
</form>

</div>
<div class="main"><br /><h2>Unsere allgemeinen Terms of Service</h2>

<p>
Damit unseren Spielern der Spaß aufgrund einiger weniger Cheater nicht vergeht, gelten für alle die nachfolgenden Terms of Service.
Überschreitet ein Benutzer die Maximalgrenze von 10 Punkten, wird ihm seine Lizenz entzogen und gegebenenfalls sein Account gesperrt.
</p>

<table>
	<colgroup>
		<col width="500">
		<col width="100">
	</colgroup>
	<tr>
		<td class="tos">Verstoß</td>
		<td class="tos">Ahndung</td>
                <td class="tos">Auto-Protection</td>
	</tr>
	<tr>
		<td class="tosnormal">Verwendung von bereits aktiviertem Lizenzschlüssel (eines anderen Spielers)</td>
		<td class="tosnormal">10 Punkte</td>
                <td class="tosnormal">Nein</td>
	</tr>
	<tr>
		<td class="tosnormal">Verwendung von ungültigem Lizenzschlüssel (Bsp. mögliche Falscheingabe)</td>
		<td class="tosnormal">1 Punkt</td>
                <td class="tosnormal">Nein</td>
	</tr>
	<tr>
		<td class="tosnormal">Verwendung von Cheat für gesteigerte Geschwindigkeit (Speedhack)</td>
		<td class="tosnormal">3 Punkte</td>
                <td class="tosnormal">Ja</td>
	</tr>
	<tr>
		<td class="tosnormal">Verwendung von Cheat für transparente Wände (Wallhack)</td>
		<td class="tosnormal">5 Punkte</td>
                <td class="tosnormal">Nein</td>
	</tr>
	<tr>
		<td class="tosnormal">Verwendung von Cheat für automatisches Zielen (Aim-Bot)</td>
		<td class="tosnormal">5 Punkte</td>
                <td class="tosnormal">Nein</td>
	</tr>
	<tr>
		<td class="tosnormal">Verwendung von Cheat für mehr Lebenspunkte</td>
		<td class="tosnormal">5 Punkte</td>
                <td class="tosnormal">Ja</td>
	</tr>
	<tr>
		<td class="tosnormal">Verwendung von Cheat für mehr Munition</td>
		<td class="tosnormal">3 Punkte</td>
                <td class="tosnormal">Ja</td>
	</tr>
	<tr>
		<td class="tosnormal">Verwendung von Cheat für Notwendigkeit des Nachlade-Mechanismus zu umgehen</td>
		<td class="tosnormal">2 Punkte</td>
                <td class="tosnormal">Ja</td>
	</tr>
	<tr>
		<td class="tosnormal">Beleidigung / Diskriminierung anderer Spieler</td>
		<td class="tosnormal">1 Punkt</td>
                <td class="tosnormal">Nein</td>
	</tr>
	<tr>
		<td class="tosnormal">Werbung im Chat</td>
		<td class="tosnormal">1 Punkt</td>
                <td class="tosnormal">Nein</td>
	</tr>
</table>

</div>
</div>
</div>
</div>
</body>
</html>
