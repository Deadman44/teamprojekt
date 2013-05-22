<?php

/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */



function check_textfieldEmpty($text)
{
	return (!empty($text));
}



function writeAllEntriesFromDB()
{

	$mysqli = @new mysqli("localhost","root","","guestbook");
	if($mysqli->connect_errno)
	{
		echo "FAIL";
	}
	else
	{
	// parametrisierte query, schutz vor sql injections
		$query = 'SELECT `NAME`,`EMAIL`,`CONTENT`,`TIMESTAMP` from entries';
		$result = $mysqli->prepare($query);
		$result->execute();
		$result->bind_result($name,$email,$content,$date);
		while($result->fetch())
		{
			writeSingleEntry($name,$email,$date,$content);
		}
	}
		
		
	$mysqli->close();
}

function writeAllUserEntriesFromDB($email)
{
	$mysqli = @new mysqli("localhost","root","","guestbook");
	if($mysqli->connect_errno)
	{
		echo "FAIL";
	}
	else
	{
		$query = "SELECT `NAME`,`EMAIL`,`CONTENT`,`TIMESTAMP` from entries where `EMAIL` = '$email'";
		$result = $mysqli->prepare($query);
		$result->execute();
		$result->bind_result($name,$email,$content,$date);
		while($result->fetch())
		{
			writeSingleEntry($name,$email,$date,$content);
		}
	}
		
		
	$mysqli->close();
}
	
	
	

function writeSingleEntry($name, $email, $date, $content)
{
	$newcontent = wordwrap($content,20,"\n",1); //nicht für die datenbank nötig
	echo "<table border =\"1\">";
	echo "<tr>";
	echo "<th> <b>Name:</b> $name </th>";
	echo "<th> <b>Email:</b> $email </th>";
	echo "<th> <b>Datum: </b> $date </th>";
	echo "</tr>";
	echo "<tr>";
	echo "<td colspan=\"3\"> $newcontent </td>";
	echo "</tr>";
	echo "</table>";
	echo "<br />";
}


function writeToDB($name, $email, $content)
{
	//Check params
	
	if(!check_email($email))
	{
		echo "<h1>Bitte korrekte Email eintragen , Post nicht angenommen!</h1>";
		return;
	}
	if(!check_textfieldEmpty($name))
	{
		echo "<h1>Bitte Namensfeld ausf&uuml;llen , Post nicht angenommen!</h1>";
		return;
	}
	if(!check_textfieldEmpty($content))
	{
		echo "<h1>Bitte Textfeld ausf&uuml;llen , Post nicht angenommen!</h1>";
		return;
	}
		
	
	
	
	$entry = new Eintrag($name,$email,$content);
	
	$mysqli = @new mysqli("localhost","root","","guestbook");
	if($mysqli->connect_errno)
	{
		echo "FAIL";
	}
	else
	{
		$dName = $entry->getName();
		$dMail = $entry->getEmail();
		$dContent = $entry->getContent();
		$dTime = $entry->getDate();
		
		
		
		$insert = 'INSERT INTO `entries`(`NAME`,`EMAIL`,`CONTENT`,`TIMESTAMP`) VALUES(?,?,?,?)';
		$eintrag = $mysqli->prepare($insert);
		$eintrag->bind_param('ssss', $dName,$dMail,$dContent,$dTime);
		$eintrag->execute();
		
		if ($eintrag->affected_rows == 1)
        {
            echo 'Der neue Eintrage wurde hinzugef&uuml;gt.';
			echo '<br />';
        }
        else
        {
            echo 'Der Eintrag konnte nicht hinzugef&uuml;gt werden.';
        }
		
		
		
	}
	
	$mysqli->close();

}


class Eintrag
{
	private $name;
	private $email;
	private $content;
	private $timestamp;
	
	function __construct($name, $email,$content)
	{
		$this->name = $name;
		$this->email = $email;
		$this->content = $content;
		
		$time = time();
		$today = date("j.n.Y",$time);
		$exactTime = date("G:i:s", $time);
		$this->timestamp = $today." ".$exactTime;
	}
	
	public function getName()
	{
		return (string) $this->name;
	}
	
	public function getDate()
	{
		return (string)$this->timestamp;
	}

	public function getEmail()
	{
		return (string)$this->email;
	}

	public function getContent()
	{
		return (string)$this->content;
	}	
	public function ausgabe()
	{
		return ("Name: $this->name, Email: $this->email, Text: $this->content, $this->timestamp");
	
	}
	
}












?>

