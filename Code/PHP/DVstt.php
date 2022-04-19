   <?php
  
   $jsonString = file_get_contents("stt/DVstt.json");
   $data = json_decode($jsonString, true);
   

    $user='abcd_ef';
    
	if ($_SERVER["REQUEST_METHOD"] == "POST") {
	
	$data['WIFI1'] = text_input($_POST["WIFI1"]);
	$data['WIFI2'] = text_input($_POST["WIFI2"]);
	$data['BLU1'] = text_input($_POST["BLU1"]);
	$data['BLU2'] = text_input($_POST["BLU2"]);
	$data['SUB'] = text_input($_POST["SUB"]);
   
	$newJsonString = json_encode($data);
	file_put_contents("stt/DVstt.json", $newJsonString);
	}

	
 function text_input($data) {
    $data = trim($data);
    $data = stripslashes($data);
    $data = htmlspecialchars($data);
    return $data;
}
   ?>  