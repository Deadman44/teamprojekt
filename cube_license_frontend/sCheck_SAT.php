<?php


include "functions.php";
$sat = $_GET['sat'];
$Qemail = $_GET['email'];

echo checkServerAccessTicket($sat, $Qemail);

?>

