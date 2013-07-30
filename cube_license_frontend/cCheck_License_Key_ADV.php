<?php
include "functions.php";
$Qemail = $_GET['email'];
$Qpass = $_GET['pass'];
$license_key = $_GET['license'];



if(check_license_key($Qemail, $Qpass, $license_key)) {
    echo "True"."\n";
    echo createAndReturnTicket($Qemail, $Qpass);
    echo 100; //erster zu hashender wert.. hier sdl.dll
} else {
	echo "False";
}
?>
