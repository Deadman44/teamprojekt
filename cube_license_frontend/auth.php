<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Strict//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd">
<html xmlns="http://www.w3.org/1999/xhtml" xml:lang="en" lang="en">
<head>
  <title>Gaming and other sins</title>
  <meta http-equiv="content-type" content="text/html;charset=utf-8" />
  <link rel="stylesheet" type="text/css" href="stylesheets/style.css" />  
</head>

<body>
<?php
     session_start();

     if (!isset($_SESSION['validLogin']) || !$_SESSION['validLogin']) {
		echo 'Bitte anmelden!';
		echo '<meta http-equiv="refresh" content="3; URL=http://localhost/index.php">';
     exit;
      }
?>
</body>
</html>