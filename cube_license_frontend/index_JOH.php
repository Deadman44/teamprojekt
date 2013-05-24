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
            addNewUserToDB("antoinette2", "marie", "test2@best.de", "bratwurst");
            checkUserLogin("test@best.de", "bratwurst");
            
            $stuff = getAllDataFromUser("test2@best.de");
            echo $stuff[0];
            echo $stuff[1];
            echo $stuff[2];
            echo $stuff[3];
            
            
        ?>
    </body>
</html>


