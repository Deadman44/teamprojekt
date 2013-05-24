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

            echo "<br>";
            echo "<br>";
            echo "<br>";
            
            $data1 = getAllDataFromUser("blaise@pascal.de");
            var_dump($data1);
            echo "<br>";
            echo "<br>";
            echo "<br>";
            $data2 = getAllDataFromUser("test2@best.de");
            var_dump($data2);
            
            
            //echo license_exists("blaise@pascal.de");

            
            
        ?>
    </body>
</html>


