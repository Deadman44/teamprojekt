<?php
include "functions.php";
$Qemail = $_GET['email'];
$Qpass = $_GET['pass'];
$license_key = $_GET['license'];



if(check_license_key($Qemail, $Qpass, $license_key)) {
	echo "True"."\n";
    echo createAndReturnTicket($Qemail, $Qpass);
} else {
	echo "False";
}
?>
