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
		$firstname = $_POST['firstname'];
		$name = $_POST['name'];
		$email = $_POST['email'];
		$password = $_POST['password'];
		$password_copy = $_POST['password_copy'];
		
		if($password==$password_copy)
		{
			// Neuen Benutzer anlegen
			addNewUserToDB($name, $firstname, $email, $password);
			echo '<meta http-equiv="refresh" content="2; URL=https://localhost/index.php">';
		}
		else
		{
			echo "Passwörter sind ungleich!";
			echo '<meta http-equiv="refresh" content="2; URL=https://localhost/index.php">';
		}
    }
?>
</head>

<body>
</body>
</html>