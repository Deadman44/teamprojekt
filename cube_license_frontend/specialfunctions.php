<?php

/*
 * diese datei darf nur vom server aus erreichbar sein, nicht von
 * außen....
 */
function setUserActive($Qemail)
{
    $credentials = getCredentialsFromFile();
    $credentialsArr = explode(":", $credentials);
    $user = $credentialsArr[0];
    $pwd = $credentialsArr[1];
    
    $active = 1;
    
    $mysqli = @new mysqli("127.0.0.1",$user,$pwd,"cube_license");
    if($mysqli->connect_errno)
    {
            echo "FAIL";
            return "SERVER_ERROR";
    }
    else
    {

            $insert = 'UPDATE USER SET ACTIVE=? WHERE email =? ';
            $eintrag = $mysqli->prepare($insert);
            $eintrag->bind_param('ss',$active,$Qemail);
            $eintrag->execute();
    }

    $mysqli->close();   
}
function setUserInActive($Qemail)
{
    $credentials = getCredentialsFromFile();
    $credentialsArr = explode(":", $credentials);
    $user = $credentialsArr[0];
    $pwd = $credentialsArr[1];
    
    $active = 0;
    
    $mysqli = @new mysqli("127.0.0.1",$user,$pwd,"cube_license");
    if($mysqli->connect_errno)
    {
            echo "FAIL";
            return "SERVER_ERROR";
    }
    else
    {

            $insert = 'UPDATE USER SET ACTIVE=? WHERE email =? ';
            $eintrag = $mysqli->prepare($insert);
            $eintrag->bind_param('ss',$active,$Qemail);
            $eintrag->execute();
    }

    $mysqli->close();   
}

function getUserActive($Qemail)
{
    $credentials = getCredentialsFromFile();
    $credentialsArr = explode(":", $credentials);
    $user = $credentialsArr[0];
    $pwd = $credentialsArr[1];
    
    $active = 1;
    
    $mysqli = @new mysqli("127.0.0.1",$user,$pwd,"cube_license");
    if($mysqli->connect_errno)
    {
            echo "FAIL";
            return "SERVER_ERROR";
    }
    else
    {

            $query = "SELECT ACTIVE from USER where EMAIL = ? ";
            $result = $mysqli->prepare($query);
            $result->bind_param('s',$Qemail);
            $result->execute();
            $result->bind_result($active);
            while($result->fetch())
            {
            }     
    }
    
    $mysqli->close();   
    return $active;

}
?>
