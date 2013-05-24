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


