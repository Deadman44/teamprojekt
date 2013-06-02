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
            
            //addNewUserToDB("ttt", "www", "tttt9@pascal.de", "horst");
            echo checkUserLogin("error@error.de", "testtesttest1");
            
            //create_license_key("tttt9@pascal.de", "false");

            echo "<br>";
            echo "<br>";
            echo "<br>";

            $lickey = "JCoH96LFUm+tNfvJUMCk8w==:sha512:1000:nz0mrnS54YbgskdiZG/GwyL+HQlvcQdzHvUDhLR9/yJSwzrG80IILVYHbACU23Q+Va4bsu/d/1OQPkGbkzIU3w==:Rh0Rng7Y0isPuWfRWkLLziVAnuYrbenDw7XJ0gqVT+9A7Zr8g+3IUGsJ0nBke8mI2h9W/K0WKYEacd9CmgmMgg==";
            echo check_license_key("error@error.de", "blabla", $lickey);
            echo "<br>";
            
            $ticket = createAndReturnTicket("error@error.de", "testtesttest1");
            echo $ticket."<br>";
            setUserActive("error@error.de", $ticket);
            echo getUserActive("error@error.de", $ticket)."<br>";
            setUserInActive("error@error.de", $ticket);
            echo getUserActive("error@error.de", $ticket)."<br>";

            
            /*
            $data1 = getAllDataFromUser("blaise@pascal.de");
            var_dump($data1);
            echo "<br>";
            echo "<br>";
            echo "<br>";
            $data2 = getAllDataFromUser("test2@best.de");
            var_dump($data2);
            */
            
            //echo license_exists("blaise@pascal.de");

            
            
        ?>
    </body>
</html>


