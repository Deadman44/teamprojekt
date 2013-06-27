<?php
include "functions.php";
$Qemail = $_GET['email'];
$ticket = $_GET['ticket'];

$result = setAndReturnServerAccessTicket($Qemail, $ticket);
echo $result;

?>
