   <?php
  
   $jsonString = file_get_contents("control/contron.json");
   $data = json_decode($jsonString, true);
   

    $user='abcd_ef';
    
	if ($_SERVER["REQUEST_METHOD"] == "POST") {
	
      $data['Pro'] = test_input($_POST["Pro"]);
      $data['Stt'] = test_input($_POST["Stt"]);
      $data['MCU'] = test_input($_POST["MCU"]);
      
      $newJsonString = json_encode($data);
      file_put_contents("control/control.json", $newJsonString);
	}

	
 function test_input($data) {
    $data = trim($data);
    $data = stripslashes($data);
    $data = htmlspecialchars($data);
    return $data;
}
   ?>  