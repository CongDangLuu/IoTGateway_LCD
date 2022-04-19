<?php
 
$servername = "localhost";
$dbname = "id18702154_iotgateway_database"; // Database name
$username = "id18702154_iotgateway";// Database user
$password = "LuanVanTotNghiep@@2022";//Database user password
 

$api_key_value = "iotgateway2021";
 
$api_key= $Name = $Stt = $ND = $DA = $AS = $Table ="";
 
date_default_timezone_set('Asia/Ho_Chi_Minh');
$timest = date('Y-m-d H:i:s'); 
 
var_dump($_POST);
 
if ($_SERVER["REQUEST_METHOD"] == "POST") {
    $api_key = test_input($_POST["api_key"]);
    if($api_key == $api_key_value) {
        $Table = test_input($_POST["Table"]);
        $Name = test_input($_POST["Name"]);
        $Stt = test_input($_POST["Stt"]);
        $ND = test_input($_POST["Temp"]);
        $DA = test_input($_POST["Humi"]);
        $AS = test_input($_POST["Light"]);

        
        
        // Create connection
        $conn = new mysqli($servername, $username, $password, $dbname);
        // Check connection
        if ($conn->connect_error) {
            die("Connection failed: " . $conn->connect_error);
        } 
        $val = " (NameDV, Status_dv, Temperature, Humidity, Light, Timestamp)
        VALUES ('$Name', '$Stt', '$ND', '$DA', '$AS', '$timest')";
        
        $sql = "INSERT INTO ";
        $sql .= $Table;
        $sql .= $val;
        
        
        if ($conn->query($sql) === TRUE) {
            echo "New record created successfully";
        } 
        else {
            echo "Error: " . $sql . "<br>" . $conn->error;
        }
    
        $conn->close();
    }
    else {
        echo "Wrong API Key provided.";
    }
 
}
else {
    echo "No data posted with HTTP POST.";
}
 
function test_input($data) {
    $data = trim($data);
    $data = stripslashes($data);
    $data = htmlspecialchars($data);
    return $data;
}