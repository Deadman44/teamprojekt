<?php
include "functions.php";
$Qemail = $_GET['email'];
$Qticket = $_GET['ticket'];
$clienthash = $_GET['hash'];



//echo permanentcheck($Qemail, $Qticket,$clienthash); //gibt antwort + neues ticket + neue hashanfrage zurück
echo permanentcheck_with_old_ticket($Qemail, $Qticket, $clienthash); //gibt antwort + altes ticket + hashwunsch zurück
?>
