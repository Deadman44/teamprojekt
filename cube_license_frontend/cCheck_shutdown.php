<?php
include "functions.php";
$Qemail = $_GET['email'];
$ticket = $_GET['ticket'];


if(getUserActive($Qemail, $ticket)) {
    setUserInActive($Qemail, $ticket);
} else {
	echo "False";
}
?>
