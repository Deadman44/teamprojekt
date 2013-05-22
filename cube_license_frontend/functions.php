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
define("PBKDF2_SALT_BYTES", 24);
define("PBKDF2_HASH_BYTES", 24);

define("HASH_SECTIONS", 4);
define("HASH_ALGORITHM_INDEX", 0);
define("HASH_ITERATION_INDEX", 1);
define("HASH_SALT_INDEX", 2);
define("HASH_PBKDF2_INDEX", 3);

function create_hash($password)
{
    // format: algorithm:iterations:salt:hash
    $salt = base64_encode(mcrypt_create_iv(PBKDF2_SALT_BYTES, MCRYPT_DEV_URANDOM));
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
    $handle = fopen("D:/xampp/credentials/database.txt","r");
    $pwd;
    $user;
    $userpw;
    
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
    //separierung von hash und salt, salt bsteht auch aus algo-nr und interations...
    $securePass = create_hash($pass);
    $secureHash = returnHashFromAll($securePass);
    $secureSalt = returnSaltFromAll($securePass);
    
    if(!check_email($email))
    {
       return false; 
    }
    
    if(!check_EmailUnique($email))
    {
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

function checkUserLogin($Qemail, $Qpass)
{
    
    $credentials = getCredentialsFromFile();
    $credentialsArr = explode(":", $credentials);
    $user = $credentialsArr[0];
    $pwd = $credentialsArr[1];
    
    $mysqli = @new mysqli("127.0.0.1",$user,$pwd,"cube_license");
    
    	if($mysqli->connect_errno)
	{
		echo "FAIL" . $user . $pwd;
	}
	else
	{
		$query = "SELECT ID,EMAIL,NNAME,VNAME,PASS,PSALT from USER where EMAIL = '$Qemail' ";
		$result = $mysqli->prepare($query);
		$result->execute();
		$result->bind_result($id,$email,$nname,$vname,$pass,$psalt);
		while($result->fetch())
		{
			//echo $id . $email . $nname . $vname . $pass . $psalt. "<br>";
		}
                
                $allPass = reconstructAll($psalt, $pass);
                if(!validate_password($Qpass, $allPass))
                {
                    echo "wrong password";
                    $mysqli->close();
                    return false;
                }
                else
                {
                    echo "succ login";
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
    
    $mysqli = @new mysqli("127.0.0.1",$user,$pwd,"cube_license");
    
    	if($mysqli->connect_errno)
	{
		echo "FAIL" . $user . $pwd;
	}
	else
	{
                $exists = 0;
		$query = "SELECT ID,EMAIL from USER where EMAIL = '$Qemail' ";
		$result = $mysqli->prepare($query);
		$result->execute();
		$result->bind_result($id,$email);
		while($result->fetch())
		{
			$exists++;
                        echo "COUNT";
		}
                
                
                if($exists >=1)
                {
                    $mysqli->close();
                    echo "email already exists";
                    return false;
                }

	}
        $mysqli->close();
        return true;
    
}



function getAllDataFromUser($Qemail)
{
    
    $credentials = getCredentialsFromFile();
    $credentialsArr = explode(":", $credentials);
    $user = $credentialsArr[0];
    $pwd = $credentialsArr[1];


    $mysqli = @new mysqli("127.0.0.1",$user,$pwd,"cube_license");
    
    	if($mysqli->connect_errno)
	{
		echo "FAIL" . $user . $pwd;
	}
	else
	{
		$query = "SELECT ID,EMAIL,NNAME,VNAME,PASS,PSALT,SKEY,HSERIAL,ACTIVE,SUSPECT from USER where EMAIL = '$Qemail' ";
		$result = $mysqli->prepare($query);
		$result->execute();
		$result->bind_result($id,$email,$nname,$vname,$pass,$psalt,$skey,$hserial,$active,$suspect);
		while($result->fetch())
		{
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

