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
            addNewUserToDB("Hoor", "Johannes", "hoorj@fh-trier.de", "bratwurst");
            checkUserLogin("hoorj@fh-trier.de", "bratwurst");
            
            
        ?>
    </body>
</html>


