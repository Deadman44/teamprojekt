<?php

include "functions.php";
$Qemail = $_GET['email'];
$amount = $_GET['amnt'];
incrementSuspects($Qemail,$amount);

?>
