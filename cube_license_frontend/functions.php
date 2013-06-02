<?php

/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 * 
 * 
 */


/*
 * Password hashing with PBKDF2.
 * Author: havoc AT defuse.ca
 * www: https://defuse.ca/php-pbkdf2.htm
 * 
 * CODE ist public domain
 * http://crackstation.net/hashing-security.htm#phpsourcecode
 * 
 */

// These constants may be changed without breaking existing hashes.
define("PBKDF2_HASH_ALGORITHM", "sha512");
define("PBKDF2_ITERATIONS", 1000);
define("PBKDF2_SALT_BYTES", 64);
define("PBKDF2_HASH_BYTES", 64);

define("HASH_SECTIONS", 4);
define("HASH_ALGORITHM_INDEX", 0);
define("HASH_ITERATION_INDEX", 1);
define("HASH_SALT_INDEX", 2);
define("HASH_PBKDF2_INDEX", 3);

function create_hash($password)
{
    // format: algorithm:iterations:salt:hash
    $salt = base64_encode(mcrypt_create_iv(PBKDF2_SALT_BYTES, MCRYPT_DEV_RANDOM));
    return PBKDF2_HASH_ALGORITHM . ":" . PBKDF2_ITERATIONS . ":" .  $salt . ":" .
        base64_encode(pbkdf2(
            PBKDF2_HASH_ALGORITHM,
            $password,
            $salt,
            PBKDF2_ITERATIONS,
            PBKDF2_HASH_BYTES,
            true
        ));
}

// modifikation von oben, vorgegebener salt
function create_hash_with_salt($password, $salt)
{
    // format: algorithm:iterations:salt:hash
    
    return PBKDF2_HASH_ALGORITHM . ":" . PBKDF2_ITERATIONS . ":" .  $salt . ":" .
        base64_encode(pbkdf2(
            PBKDF2_HASH_ALGORITHM,
            $password,
            $salt,
            PBKDF2_ITERATIONS,
            PBKDF2_HASH_BYTES,
            true
        ));
}

function check_license_key($Qemail,$Qpass,$license_key) //mit password für user, da man ansonsten geklaute lizenz mit emailliste 
        //vergleichen kann, hat man einen treffer, muss man nur noch password stehlen
{

    if(!checkUserLogin($Qemail, $Qpass))
    {
        echo "Wrong Login";
        return false;
    }
    
    $credentials = getCredentialsFromFile();
    $credentialsArr = explode(":", $credentials);
    $user = $credentialsArr[0];
    $pwd = $credentialsArr[1];
    
    $lkeyArr = explode(":", $license_key);
    $random = $lkeyArr[0]; //die vom user gegeben random zahl
    $hashed_random = $lkeyArr[1]; //test... //die hmac, also random und salt;; 
       
    
    $mysqli = @new mysqli("127.0.0.1",$user,$pwd,"cube_license");
    
    $skey = "NO KEY ERROR";
    $hserial ="NO SERIAL ERROR";
    if($mysqli->connect_errno)
    {
            //echo "FAIL" . $user . $pwd;
            return false;
    }
    else
    {
            $query = "SELECT SKEY,HSERIAL from USER where EMAIL = ? ";
            $result = $mysqli->prepare($query);
            $result->bind_param('s',$Qemail);
            $result->execute();
            $result->bind_result($skey,$hserial);
            while($result->fetch())
            {
                    //noch keine fehlerüberprüfung...
            }     

    }
    $mysqli->close();
    

    /*
     * erstes überprüfung kann umgangen werden, fall an eine gültige lizenz ein doppelpunkt
     * angehängt wird, die zweite überprüfung schlägt dann aber fehl!
     * deswegen beide wichtig
     * wobei der angesprochene angriff bedingt, dass man den originalkey hat
     * ggfls muss bei eingaben auf doppelpunkt gefiltert werden...
     */
    $hashed_random_candidate = explode(':',create_hash_with_salt($random, $skey))[3];
    if($hashed_random_candidate != $hashed_random)
    {
        //echo $hashed_random . " ORIGINAL" . "<br>";
        //echo $hashed_random_candidate . " GESENDET" . "<br>";
        //echo "SERIAL WRONG -- hashed_random_error" .  "<br>";
        return false;
    }
    
    $hserial_candidate = hash("sha512",$license_key);
    
    if($hserial_candidate != $hserial)
    {
        //echo $hserial_candidate . " gesendet ". " <br>";
        //echo $hserial . " original " . " <br>";
        //echo "Serial Wrong -- hserial error";
        return false;
    }
    
    return true;
    
}

function get_hserial($Qemail)
{
    $credentials = getCredentialsFromFile();
    $credentialsArr = explode(':', $credentials);
    $user = $credentialsArr[0];
    $pwd = $credentialsArr[1];

    $hserial ="nofetched";

    $mysqli = @new mysqli("127.0.0.1",$user,$pwd,"cube_license");
    
    	if($mysqli->connect_errno)
	{
		echo "FAIL" . $user . $pwd;
	}
	else
	{
		$query = 'SELECT HSERIAL from USER where email=? ';
		$result = $mysqli->prepare($query);
                $result->bind_param('s',$Qemail);
		$result->execute();

		$result->bind_result($hserial);
		while($result->fetch())
		{
			
		}  
	}
        $mysqli->close();
        return $hserial;
    
}

function license_exists($Qemail)
{

    if(get_hserial($Qemail) != null)
    {
        //echo get_hserial($Qemail);
        return true;
    }
    
    //echo "license not available";
    return false;
    
    
}

// eigene fkt
function create_license_key($Qemail,$forgotten_license)
{
    $forgotten_license = "NOINTEREST";
    
    if( license_exists($Qemail)) //  && $forgotten_license == "false")
    {
        echo "Lizenzkey existiert bereits" ;
        return false;
    }
    
    $random = base64_encode(mcrypt_create_iv(16, MCRYPT_DEV_RANDOM)); // 128 bit random
    $salt = base64_encode(mcrypt_create_iv(PBKDF2_SALT_BYTES, MCRYPT_DEV_RANDOM)); 
    $hashed_random = explode(":",create_hash_with_salt($random,$salt));
     
    $license_key = $random.":".$hashed_random[3]; //nur hashed_random ausgeben, kein salt und params
    $hserial = hash("sha512", $license_key);
    
    $credentials = getCredentialsFromFile();
    $credentialsArr = explode(":", $credentials);
    $user = $credentialsArr[0];
    $pwd = $credentialsArr[1];
    
    $mysqli = @new mysqli("127.0.0.1",$user,$pwd,"cube_license");
    if($mysqli->connect_errno)
    {
            echo "FAIL";
            return "SERVER_ERROR";
    }
    else
    {

            $insert = 'UPDATE USER SET skey=?, hserial=? WHERE email =? ';
            $eintrag = $mysqli->prepare($insert);
            $eintrag->bind_param('sss',$salt,$hserial,$Qemail);
            $eintrag->execute();
            //echo "<br>";
            //echo "<br>";
            //echo "<br>";
            //echo $license_key;
            //echo "<br>";
            //echo "<br>";
            //echo "<br>";
            
            return $license_key;

    }


    //return $license_key . "ERRRRRRRRR"; //temporär
    $mysqli->close();
    




}

//eigene Funktion
function returnSaltFromAll($entry)
{
    $params = explode(":",$entry);
    $saltBig = $params[0].":".$params[1].":".$params[2];
    return $saltBig;
}

//eigene Funktion
function returnHashFromAll($entry)
{
    $params = explode(":",$entry);
    return $params[3];
}
//eigene fkt
function reconstructAll($saltBig,$hash)
{
    return $saltBig.":".$hash;
}

function getCredentialsFromFile()
{
    $handle = fopen("C:/w/cube_license_frontend/credentials/database.txt","r");
    
    //eine zeile lesen, die oberste
    $buffer = fgets($handle);
    $userpw = explode(":", $buffer);
    $user = $userpw[0];
    $pwd = $userpw[1];
    fclose($handle);
    
    return $user.":".$pwd;
}

//eigene ftk, fuegt neuen user zu db hinzu
function addNewUserToDB($nname, $vname, $email, $pass)
{
    //separierung von hash und salt, salt bsteht auch aus algo-nr und iterations...
    $securePass = create_hash($pass);
    $secureHash = returnHashFromAll($securePass);
    $secureSalt = returnSaltFromAll($securePass);
    
    if(!check_email($email))
    {
       echo "<h3> EMAIL im falschen Format </h3>";
       return false; 
    }
    
    if(!check_EmailUnique($email))
    {
        echo "<h3> EMAIL existiert bereits </h3>";
       return false; 
    }
	
    if(!check_password($pass))
    {
        echo "<h3> Passwort entspricht nicht den Richtlinien </h3>";
        return false;
    }
    
    
    $credentials = getCredentialsFromFile();
    $credentialsArr = explode(":", $credentials);
    $user = $credentialsArr[0];
    $pwd = $credentialsArr[1];
    

    
    $mysqli = @new mysqli("127.0.0.1",$user,$pwd,"cube_license");
	if($mysqli->connect_errno)
	{
		echo "FAIL";
	}
	else
	{
		
		$insert = 'INSERT INTO USER(NNAME, VNAME, EMAIL, PASS, PSALT) VALUES(?,?,?,?,?)';
		$eintrag = $mysqli->prepare($insert);
		$eintrag->bind_param('sssss',$nname, $vname,$email,$secureHash,$secureSalt);
		$eintrag->execute();
		
		if ($eintrag->affected_rows == 1)
        {
            echo '<h3>Der neue Eintrage wurde hinzugef&uuml;gt.</h3> <br />';
        }
        else
        {
            echo '<h3>Der Eintrag konnte nicht hinzugef&uuml;gt werden.</h3>';
        }	
	}
	
	$mysqli->close();
}

function checkUserLogin($Qemail, $Qpass)
{
    
    $credentials = getCredentialsFromFile();
    $credentialsArr = explode(":", $credentials);
    $user = $credentialsArr[0];
    $pwd = $credentialsArr[1];
    
    
    $id = "null";
    $email = "null";
    $nname = "null";
    $vname = "null";
    $pass = "null";
    $psalt = "null";

    
    
    $mysqli = @new mysqli("127.0.0.1",$user,$pwd,"cube_license");
    
    	if($mysqli->connect_errno)
	{
		echo "FAIL" . $user . $pwd;
	}
	else
	{
		$query = "SELECT ID,EMAIL,NNAME,VNAME,PASS,PSALT from USER where EMAIL = ? ";
		$result = $mysqli->prepare($query);
                $result->bind_param('s',$Qemail);
		$result->execute();
		$result->bind_result($id,$email,$nname,$vname,$pass,$psalt);
		while($result->fetch())
		{
			//echo $id . $email . $nname . $vname . $pass . $psalt. "<br>";
		}
                
                $allPass = reconstructAll($psalt, $pass);
                if(!validate_password($Qpass, $allPass))
                {
                    //echo "wrong password";
                    $mysqli->close();
                    return false;
                }
                else
                {
                    //echo "succ login";
                }
                
        
	}
        $mysqli->close();
        return true;
}


function check_email($email)
{        
    if(preg_match('/^[^\x00-\x20()<>@,;:\\".[\]\x7f-\xff]+(?:\.[^\x00-\x20()<>@,;:\\".[\]\x7f-\xff]+)*\@[^\x00-\x20()<>@,;:\\".[\]\x7f-\xff]+(?:\.[^\x00-\x20()<>@,;:\\".[\]\x7f-\xff]+)+$/i', $email))
	{
            return true;
	}
	else
	{
	    return false;
	}
}

function check_EmailUnique($Qemail)
{
    $credentials = getCredentialsFromFile();
    $credentialsArr = explode(":", $credentials);
    $user = $credentialsArr[0];
    $pwd = $credentialsArr[1];
    $id = "noid";
    $email ="noemail";
    
    $mysqli = @new mysqli("127.0.0.1",$user,$pwd,"cube_license");
    
    	if($mysqli->connect_errno)
	{
		echo "FAIL" . $user . $pwd;
	}
	else
	{
                $exists = 0;
		$query = "SELECT ID,EMAIL from USER where EMAIL = ? ";
		$result = $mysqli->prepare($query);
                $result->bind_param('s',$Qemail);
		$result->execute();
		$result->bind_result($id,$email);
		while($result->fetch())
		{
			$exists++;
		}
                
                
                if($exists >=1)
                {
                    $mysqli->close();
                    //echo "email already exists";
                    return false;
                }

	}
        $mysqli->close();
        return true;
    
}


function check_password($password)
{
	if(check_password_length($password) && check_password_number_included($password))
	{
		return true;
	}
	else
	{
		return false;
	}
}

function check_password_length($password)
{
	if(strlen($password) >= 10)
	{
		return true;
	}
	else
	{
		echo '<h3> Passwortlänge zu kurz, bitte erneut registrieren. </h3>';
		return false;
	}
}

function check_password_number_included($password)
{
	if(preg_match( "/\d+/", $password ))
	{
		return true;
	}
	else
	{
		echo '<h3>Keine Zahlen enthalten, bitte erneut registrieren.</h3>';
		return false;
	}
}


function change_Password($Qemail,$oldpass, $pass)
{
    if(!checkUserLogin($Qemail, $oldpass))
    {
        echo "<h3> Aktuelles Passwort nicht korrekt </h3>";
        return false;
    }
    if(!check_password($pass))
    {
        echo "<h3>Bitte anderes Passwort eingeben</h3>";
    }
    
    $securePass = create_hash($pass);
    $secureHash = returnHashFromAll($securePass);
    $secureSalt = returnSaltFromAll($securePass);
    
    $credentials = getCredentialsFromFile();
    $credentialsArr = explode(":", $credentials);
    $user = $credentialsArr[0];
    $pwd = $credentialsArr[1];
    
    
    $mysqli = @new mysqli("127.0.0.1",$user,$pwd,"cube_license");
    if($mysqli->connect_errno)
    {
            echo "FAIL";
            return "SERVER_ERROR";
    }
    else
    {
            $insert = 'UPDATE USER SET PASS = ?,PSALT=? WHERE email =? ';
            $eintrag = $mysqli->prepare($insert);
            $eintrag->bind_param('sss',$secureHash,$secureSalt,$Qemail);
            $eintrag->execute();
    }

    $mysqli->close();   
}

function getAllDataFromUser($Qemail)
{
    
    if(check_EmailUnique($Qemail))
    {
        echo " NO USER FOR REQUEST";
        return false;
        
    }
    $credentials = getCredentialsFromFile();
    $credentialsArr = explode(":", $credentials);
    $user = $credentialsArr[0];
    $pwd = $credentialsArr[1];

    $id = "null";
    $email = "null";
    $nname = "null";
    $vname = "null";
    $pass = "null";
    $psalt = "null";
    $skey = "null";
    $hserial = "null";
    $active = "null";
    $suspect = "null";

    $mysqli = @new mysqli("127.0.0.1",$user,$pwd,"cube_license");
    
    	if($mysqli->connect_errno)
	{
		echo "FAIL" . $user . $pwd;
	}
	else
	{
		$query = "SELECT ID,EMAIL,NNAME,VNAME,PASS,PSALT,SKEY,HSERIAL,ACTIVE,SUSPECT from USER where EMAIL = ? ";
		$result = $mysqli->prepare($query);
                $result->bind_param('s',$Qemail);
		$result->execute();
		$result->bind_result($id,$email,$nname,$vname,$pass,$psalt,$skey,$hserial,$active,$suspect);
		while($result->fetch())
		{
                        $psalt = explode(":", $psalt)[2];    //nur salt nehmen, nicht zusatzinfos                        
			$all = $id .":". $email .":". $nname .":". $vname .":". $pass .":". $psalt .":". $skey .":". $hserial . ":".$active.":". $suspect;
		}
                
                
                $allArr = explode(":", $all);
                
        
	}
        $mysqli->close();
        return $allArr;
}


function validate_password($password, $good_hash)
{
    $params = explode(":", $good_hash);
    if(count($params) < HASH_SECTIONS)
       return false;
    $pbkdf2 = base64_decode($params[HASH_PBKDF2_INDEX]);
    return slow_equals(
        $pbkdf2,
        pbkdf2(
            $params[HASH_ALGORITHM_INDEX],
            $password,
            $params[HASH_SALT_INDEX],
            (int)$params[HASH_ITERATION_INDEX],
            strlen($pbkdf2),
            true
        )
    );
}

// Compares two strings $a and $b in length-constant time.
function slow_equals($a, $b)
{
    $diff = strlen($a) ^ strlen($b);
    for($i = 0; $i < strlen($a) && $i < strlen($b); $i++)
    {
        $diff |= ord($a[$i]) ^ ord($b[$i]);
    }
    return $diff === 0;
}

/*
 * PBKDF2 key derivation function as defined by RSA's PKCS #5: https://www.ietf.org/rfc/rfc2898.txt
 * $algorithm - The hash algorithm to use. Recommended: SHA256
 * $password - The password.
 * $salt - A salt that is unique to the password.
 * $count - Iteration count. Higher is better, but slower. Recommended: At least 1000.
 * $key_length - The length of the derived key in bytes.
 * $raw_output - If true, the key is returned in raw binary format. Hex encoded otherwise.
 * Returns: A $key_length-byte key derived from the password and salt.
 *
 * Test vectors can be found here: https://www.ietf.org/rfc/rfc6070.txt
 *
 * This implementation of PBKDF2 was originally created by https://defuse.ca
 * With improvements by http://www.variations-of-shadow.com
 */
function pbkdf2($algorithm, $password, $salt, $count, $key_length, $raw_output = false)
{
    $algorithm = strtolower($algorithm);
    if(!in_array($algorithm, hash_algos(), true))
        die('PBKDF2 ERROR: Invalid hash algorithm.');
    if($count <= 0 || $key_length <= 0)
        die('PBKDF2 ERROR: Invalid parameters.');

    $hash_length = strlen(hash($algorithm, "", true));
    $block_count = ceil($key_length / $hash_length);

    $output = "";
    for($i = 1; $i <= $block_count; $i++) {
        // $i encoded as 4 bytes, big endian.
        $last = $salt . pack("N", $i);
        // first iteration
        $last = $xorsum = hash_hmac($algorithm, $last, $password, true);
        // perform the other $count - 1 iterations
        for ($j = 1; $j < $count; $j++) {
            $xorsum ^= ($last = hash_hmac($algorithm, $last, $password, true));
        }
        $output .= $xorsum;
    }

    if($raw_output)
        return substr($output, 0, $key_length);
    else
        return bin2hex(substr($output, 0, $key_length));
}


/*
 * Special Authentication Functions
 * 
 */

function setUserActive($Qemail,$ticket)
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

            $insert = 'UPDATE USER SET ACTIVE=? WHERE email =? AND TICKET=?';
            $eintrag = $mysqli->prepare($insert);
            $eintrag->bind_param('sss',$active,$Qemail,$ticket);
            $eintrag->execute();
    }

    $mysqli->close();   
}
function setUserInActive($Qemail,$ticket)
{
    $credentials = getCredentialsFromFile();
    $credentialsArr = explode(":", $credentials);
    $user = $credentialsArr[0];
    $pwd = $credentialsArr[1];
    
    $active = 0;
    $newticket = "empty";
    
    $mysqli = @new mysqli("127.0.0.1",$user,$pwd,"cube_license");
    if($mysqli->connect_errno)
    {
            echo "FAIL";
            return "SERVER_ERROR";
    }
    else
    {

            $insert = 'UPDATE USER SET ACTIVE=?, TICKET=? WHERE email =? AND TICKET=? ';
            $eintrag = $mysqli->prepare($insert);
            $eintrag->bind_param('ssss',$active,$newticket,$Qemail,$ticket);
            $eintrag->execute();
    }

    $mysqli->close();   
}

function getUserActive($Qemail,$ticket)
{
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

            $query = "SELECT ACTIVE from USER where EMAIL = ? AND TICKET=?";
            $result = $mysqli->prepare($query);
            $result->bind_param('ss',$Qemail,$ticket);
            $result->execute();
            $result->bind_result($active);
            while($result->fetch())
            {
            }     
    }
    
    $mysqli->close();   
    return $active;

}


function createAndReturnTicket($Qemail,$Qpass)
{
    $credentials = getCredentialsFromFile();
    $credentialsArr = explode(":", $credentials);
    $user = $credentialsArr[0];
    $pwd = $credentialsArr[1];
    
    if(!checkUserLogin($Qemail, $Qpass))
    {
        return false;
    }
    
    $hashAndSalt = create_hash($Qemail.$Qpass);
    $temporaryticket = returnHashFromAll($hashAndSalt); //Salt implizit, gibt nur Hash aus     
    $mysqli = @new mysqli("127.0.0.1",$user,$pwd,"cube_license");
    if($mysqli->connect_errno)
    {
            echo "FAIL";
            return "SERVER_ERROR";
    }
    else
    {

            $insert = 'UPDATE USER SET TICKET=? WHERE email =? ';
            $eintrag = $mysqli->prepare($insert);
            $eintrag->bind_param('ss',$temporaryticket,$Qemail);
            $eintrag->execute();
    }

    $mysqli->close();
    
    setUserActive($Qemail, $temporaryticket);  
    return $temporaryticket;
}

function createTicketWithOldTicket($Qemail,$ticket)
{
    
    $credentials = getCredentialsFromFile();
    $credentialsArr = explode(":", $credentials);
    $user = $credentialsArr[0];
    $pwd = $credentialsArr[1];
       
    $hashAndSalt = create_hash($Qemail.$ticket);
    $temporaryticket = returnHashFromAll($hashAndSalt); //Salt implizit, gibt nur Hash aus     
    $mysqli = @new mysqli("127.0.0.1",$user,$pwd,"cube_license");
    if($mysqli->connect_errno)
    {
            echo "FAIL";
            return "SERVER_ERROR";
    }
    else
    {

            $insert = 'UPDATE USER SET TICKET=? WHERE email =? AND ticket=? ';
            $eintrag = $mysqli->prepare($insert);
            $eintrag->bind_param('sss',$temporaryticket,$Qemail,$ticket);
            $eintrag->execute();
    }

    $mysqli->close();   
    return $temporaryticket;
    
}

function permanentcheck($Qemail,$oldticket)
{
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
            $query = "SELECT ACTIVE,TICKET from USER where EMAIL = ? AND TICKET=?";
            $result = $mysqli->prepare($query);
            $result->bind_param('ss',$Qemail,$oldticket);
            $result->execute();
            $result->bind_result($active,$ticket);
            while($result->fetch())
            {
            }     
    }
    
    if(!$active)
    {
        return "false";
    }
    if(strcmp($oldticket, $ticket) != 0)
    {
        return "false";
    }
    
    $ticket = createTicketWithOldTicket($Qemail, $ticket);
    
    
    $mysqli->close();   
    return $ticket;
}
