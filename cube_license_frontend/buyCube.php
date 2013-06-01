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

  <?php
	require('functions.php');
	echo '<p> Lizenzkey wurde generiert: </p>';
	echo '<textarea name="Lizenzkey" cols="50" rows="10">';
	echo create_license_key($_SESSION['user'],false);
	echo '</textarea>';	
	?>
	
</div>
</div>
</div>
</div>
</body>
</html>
