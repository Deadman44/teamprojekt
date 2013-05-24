<<<<<<< HEAD
<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Strict//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd">
<html xmlns="http://www.w3.org/1999/xhtml" xml:lang="en" lang="en">
<head>
  <title>Gaming and other sins</title>
  <meta http-equiv="content-type" content="text/html;charset=utf-8" />
  <link rel="stylesheet" type="text/css" href="stylesheets/style.css" />  
</head>
=======
<!DOCTYPE html>
<html>
    <head>
        <meta http-equiv="Content-Type" content="text/html; charset=UTF-8">
        <title></title>
    </head>
    <body>
        <?php
            include 'functions.php';
            include 'dbtests.php';
            

            
            $lkey = create_license_key("lolol@fh-trier.de");
            echo " --> KEY <br>".$lkey."<br>";
            check_license_key("lolol@fh-trier.de", "bratwurst", $lkey);
            
  
        ?>
    </body>
</html>
>>>>>>> 3ee75d6a9704c9238bb4b61470eb28d1a09baf50

<body>
<div class="mySite">
<div class="oben"></div>
<div class="menue">
<ul>
<li class="menue"><a href="index.html">Home</a></li>
<li class="menue"><a href="games.html">Games</a></li>
<li class="menue"><a href="community.html">Community</a></li>
<li class="menue"><br /></li>
<li class="menue"><a href="impressum.html">Impressum</a></li>
</ul>
</div>
<div class="rahmenMain">
<div class="anmeldung">
<div class="anmeldemain"> <p>Anmeldemain</p></div>
<div class="anmeldebox">

<form action="login.php" method="post">
<p class="anmeld">
Benutzername:&nbsp; <input type="text" name="user" value="" ></input>
Passwort:&nbsp; <input type="password" name="password" value="" ></input>
<input type="submit" value="Login"/>
</p>

</form>

</div>
<div class="main"><br /><h2>Alles was dein Gamerherz höher schlagen lässt..</h2><br />
<p></p>
</div>
</div>
</div>
</div>
</body>
</html>
