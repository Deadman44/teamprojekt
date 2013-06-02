<?php
include "functions.php";
$Qemail = $_GET['email'];
$Qticket = $_GET['ticket'];



echo permanentcheck($Qemail, $Qticket);
?>
