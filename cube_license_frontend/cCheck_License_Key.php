<?php

$Qemail = $_GET['email'];
$Qpass = $_GET['pass'];
$license_key = $_GET['license'];



check_license_key($Qemail, $Qpass, $license_key);
?>
