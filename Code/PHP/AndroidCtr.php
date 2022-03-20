   <?php
  
   $jsonString = file_get_contents("test/test.json");
   $data = json_decode($jsonString, true);
   

    $user='abcd_ef';
    
	if ($_SERVER["REQUEST_METHOD"] == "POST") {
	
	$data['Pro'] = test_input($_POST["Pro"]);
	$data['Stt'] = test_input($_POST["Stt"]);
   
	$newJsonString = json_encode($data);
	file_put_contents("test/test.json", $newJsonString);
	}

	
 function test_input($data) {
    $data = trim($data);
    $data = stripslashes($data);
    $data = htmlspecialchars($data);
    return $data;
}
   ?>  