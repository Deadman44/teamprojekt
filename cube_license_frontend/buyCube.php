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

<?php
	//Button wurde sonst kurz angezeigt, wenn Meldung zum Login erfolgte.
	if (isset($_SESSION['validLogin']) ) {
		echo '<form action="logout.php"> <input type="submit" value="Logout"></input> </form>';
	}
?>

</div>
<div class="main">
<h1> Lizenzkey </h1>
<p> Es ist wichtig den Lizenzkey sofort zu speichern.
Bitte kopieren Sie den Key in eine Textdatei mit Namen "Lizenzschluessel.txt".
Speichern Sie diese im Cube-Verzeichnis mit der Datei "cube.bat", diese sollte sich im Ordner "../cube/Cube" befinden.
</p></ br>
  <?php
	require('functions.php');
	echo '<h2> Lizenzkey wurde generiert: </h2>';
	echo '<textarea name="Lizenzkey" cols="50" rows="10">';
	echo create_license_key($_SESSION['user'],false);
	echo '</textarea>';	
	?>
	
<h2>Eindrücke des Games..</h2> 
<p>
<a href="images/screenshot_59277.jpg"><img src="images/screenshot_59277_thumb.jpg" width="160" height="120" border=0 alt="click to enlarge"></a>
<a href="images/screenshot_129601.jpg"><img src="images/screenshot_129601_thumb.jpg" width="160" height="120" border=0 alt="click to enlarge"></a>
<a href="images/screenshot_802899.jpg"><img src="images/screenshot_802899_thumb.jpg" width="160" height="120" border=0 alt="click to enlarge"></a>
<p>
<a href="images/screenshot_522827.jpg"><img src="images/screenshot_522827_thumb.jpg" width="160" height="120" border=0 alt="click to enlarge"></a>
<a href="images/screenshot_828390.jpg"><img src="images/screenshot_828390_thumb.jpg" width="160" height="120" border=0 alt="click to enlarge"></a>
<a href="images/screenshot_1027650.jpg"><img src="images/screenshot_1027650_thumb.jpg" width="160" height="120" border=0 alt="click to enlarge"></a>
<p>
<a href="images/screenshot_1478660.jpg"><img src="images/screenshot_1478660_thumb.jpg" width="160" height="120" border=0 alt="click to enlarge"></a>
<a href="images/screenshot_1675435.jpg"><img src="images/screenshot_1675435_thumb.jpg" width="160" height="120" border=0 alt="click to enlarge"></a>
<a href="images/screenshot_1914089.jpg"><img src="images/screenshot_1914089_thumb.jpg" width="160" height="120" border=0 alt="click to enlarge"></a>
<p>
</div>
</div>
</div>
</div>
</body>
</html>
