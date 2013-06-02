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
            
            //addNewUserToDB("ttt", "www", "wurst@brot.de", "testtesttest1");
            //echo checkUserLogin("wurst@brot.de", "testtesttest1");
            
            //create_license_key("tttt9@pascal.de", "false");

            $serial = "30hRMWC83LBd2Ly+67AFKQ==:Q9cy9Uvd4Na76ddlgJCNLcOyDC4MnWoB0SVwg67ydeRQ0sO4jFN+D48DturQLYYdUK5G3CWIASbsQOplwFkhzg==";
            $serial2 = "2IxrRyNJwqUVFXrDysl5Vw==:Icj4zeBMZ2fVzIPKo2zFpG3XOK/Tm2E88MLmLeDbGeYgl1AtRAPfyYcGpHWcyQVh2gyD+OWf28lpygudSURDWg==";
            echo "<br>";
            echo "<br>";
            echo "<br>";
            echo checkUserLogin("test@test.de", "testtesttest1");
            echo "<br>";
            
            echo check_license_key("test@test.de", "testtesttest1", $serial);
            

            
            
            /*
            $ticket = createAndReturnTicket("error@error.de", "testtesttest1");
            echo $ticket."<br>";
            setUserActive("error@error.de", $ticket);
            echo getUserActive("error@error.de", $ticket)."<br>";
            setUserInActive("error@error.de", $ticket);
            echo getUserActive("error@error.de", $ticket)."<br>";

*/
            
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


