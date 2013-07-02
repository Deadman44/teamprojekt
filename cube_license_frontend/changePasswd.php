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
  
<?php
	require('functions.php');
    if ($_SERVER['REQUEST_METHOD'] == 'POST')
	{	
  		$password = $_POST['password'];
		$password = htmlentities($password);
		$new_password = $_POST['new_password'];
		$new_password = htmlentities($new_password);
		$new_password_copy = $_POST['new_password_copy'];
		$new_password_copy = htmlentities($new_password_copy);
		
		$user = $_SESSION['user'];

		if($new_password==$new_password_copy)
		{
			if(change_Password($user, $password, $new_password)){
				echo "<h3 class=\"meldung\">Passwort wurde geändert!</h3>";
				echo '<meta http-equiv="refresh" content="2; URL=https://localhost/userIndex.php">';
			}
			else
			{
				echo "<h3 class=\"meldung\">Fehlerhafte Eingabe, bitte erneut versuchen!</h3>";
				echo '<meta http-equiv="refresh" content="2; URL=https://localhost/userData.php">';
			}
		}
		else
		{
			echo "<h3 class=\"meldung\">Die neuen Passwörter sind ungleich, bitte erneut versuchen!</h3>";
			echo '<meta http-equiv="refresh" content="2; URL=https://localhost/userData.php">';
		}
	}
?>
  
  
</head>

<body>
</body>
</html>