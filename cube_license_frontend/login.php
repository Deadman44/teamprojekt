<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Strict//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd">
<html xmlns="http://www.w3.org/1999/xhtml" xml:lang="en" lang="en">
<head>
  <title>Gaming and other sins</title>
  <meta http-equiv="content-type" content="text/html;charset=utf-8" />
  <link rel="stylesheet" type="text/css" href="stylesheets/style.css" />  
  <?php
	require('functions.php');
	
    if ($_SERVER['REQUEST_METHOD'] == 'POST') {
		session_start();
		
		$user = $_POST['user'];
		$user = htmlentities($user);
		$password = $_POST['password'];
		$password = htmlentities($password);
		
		$_SESSION['user'] = $user;
		// Benutzername und Passwort werden überprüft
		if(checkUserLogin($user, $password)) {
			$_SESSION['validLogin'] = true;
			echo '<meta http-equiv="refresh" content="0; URL=https://localhost/userIndex.php">';
                        //echo '<meta http-equiv="refresh" content="0; URL=https://25.102.49.21/userIndex.php">';
			exit();
		} else {
			$_SESSION['validLogin'] = false;
			echo "<h3 class=\"meldung\">Falsche Login-Daten!</h3>";
			echo '<meta http-equiv="refresh" content="4; URL=https://localhost/index.php">';
		}
    }
?>
</head>

<body>
</body>
</html>