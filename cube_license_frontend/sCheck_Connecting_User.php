<?php
include "functions.php";
$currentTicket = $_GET['ticket'];

$result =  getEmailByTicket($currentTicket);
if(strcmp($result,"null") == 0)
{
    echo "False";
}
else
{
    echo $result;
}

?>
