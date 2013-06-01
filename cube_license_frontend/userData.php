<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Strict//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd">
<html xmlns="http://www.w3.org/1999/xhtml" xml:lang="en" lang="en">

  <?php
	require('auth.php');
  ?>

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
<li class="menue"><a href="userIndex.php">Home</a></li>
<li class="menue"><a href="userGames.php">Games</a></li>
<li class="menue"><a href="userData.php">Change Userdata</a></li>
</ul>
</div>
<div class="rahmenMain">
<div class="anmeldung">
<div class="anmeldemain"></div>
<div class="anmeldebox">
<form action="logout.php">
<input type="submit" value="Logout"></input>
</form>
</div>
<div class="main"><br /><h2>Hier besteht die Möglichkeit das Passwort zu verändern.. </h2>

<form action="changePasswd.php" method="post"><p class="anmeld"> 
<table>
	<tr>
		<td> Altes Passwort:&nbsp;</td>
		<td><input type="password" name="password" value="" /></td>
	</tr>
	<tr>
		<td> Neues Passwort:&nbsp;</td>
		<td><input type="password" name="new_password" value="" /></td>
		<td> (Mind. 10 Zeichen + Zahl enthalten.)</td>
	</tr>
	<tr>
		<td> Wdh. neues Passwort:&nbsp;</td>
		<td><input type="password" name="new_password_copy" value="" /></td>
		<td> (Passwort erneut eingeben)</td>
	</tr>
</table>
<input type="submit" value="Passwort ändern"/></p>
</form>




</div>
</div>
</div>
</div>
</body>
</html>