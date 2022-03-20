   <?php
  
   $jsonString = file_get_contents("stt/stt.json");
   $data = json_decode($jsonString, true);
   

    $user='abcd_ef';
    
	if ($_SERVER["REQUEST_METHOD"] == "POST") {
	
	$data['WIF1'] = text_input($_POST["WIF1"]);
	$data['WIF2'] = text_input($_POST["WIF2"]);
	$data['BLU1'] = text_input($_POST["BLU1"]);
	$data['BLU2'] = text_input($_POST["BLU2"]);
	$data[''] = text_input($_POST["SUB"]);
   
	$newJsonString = json_encode($data);
	file_put_contents("stt/stt.json", $newJsonString);
	}

	
 function text_input($data) {
    $data = trim($data);
    $data = stripslashes($data);
    $data = htmlspecialchars($data);
    return $data;
}
   ?>  