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
            
            addNewUserToDB("blaise", "pascal", "blaise@pascal.de", "horst");
            checkUserLogin("blaise@pascal.de", "horst");
            
            create_license_key("blaise@pascal.de", "false");

            
            echo license_exists("homo@fh-trier.de");

            
            
        ?>
    </body>
</html>


