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
            
            
            //echo create_hash("HELLO");
            //addNewUserToDB("feilen", "markus", "homo@fh-trier.de", "bratwurst");

            //checkUserLogin("homo@fh-trier.de", "bratwurst");
            
            $lkey = create_license_key("homo@fh-trier.de");
            check_license_key("homo@fh-trier.de", "bratwurst", $lkey);
            

            
            
        ?>
    </body>
</html>


