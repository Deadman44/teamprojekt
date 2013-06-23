<?php

function readFileData(){
	$datei = implode("<br>",file("gameData.txt"));
	//echo $datei;
}

function dropActive($Qemail,$ticket){
	while(true){

		$credentials = getCredentialsFromFile();
		$credentialsArr = explode(":", $credentials);
		$user = $credentialsArr[0];
		$pwd = $credentialsArr[1];
    
		$active = "ERROR NOT FOUND";
    
		$mysqli = @new mysqli("127.0.0.1",$user,$pwd,"cube_license");
		if($mysqli->connect_errno)
		{
				echo "FAIL";
				return "SERVER_ERROR";
		}
		else
		{

            $query = "UPDATE USER set ACTIVE=0 where EMAIL=? AND TICKET=?";
            $result = $mysqli->prepare($query);
            $result->bind_param('ss',$Qemail,$ticket);
            $result->execute();
            $result->bind_result($active);
            while($result->fetch())
            {
            }     
		}
		$mysqli->close();   

		sleep(60*5);	// 300 Sekunden warten
	}

}
?>
