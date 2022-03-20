<?php
 
$servername = "localhost";
$dbname = "id18138828_gateway_database"; // Database name
$username = "id18138828_iotgateway";// Database user
$password = "Q]sac8*&Kq)3#M{c";//Database user password
 

$api_key_value = "iotgateway2021";
 
$api_key= $ten = $Stt = $nhietdo = $doam = $anhsang = $table ="";
 
date_default_timezone_set('Asia/Ho_Chi_Minh');
$timest = date('Y-m-d H:i:s'); 
 
var_dump($_POST);
 
if ($_SERVER["REQUEST_METHOD"] == "POST") {
    $api_key = test_input($_POST["api_key"]);
    if($api_key == $api_key_value) {
        $table = test_input($_POST["table"]);
        $ten = test_input($_POST["ten"]);
        $Stt = test_input($_POST["Stt"]);
        $nhietdo = test_input($_POST["nhietdo"]);
        $doam = test_input($_POST["doam"]);
        $anhsang = test_input($_POST["anhsang"]);

        
        
        // Create connection
        $conn = new mysqli($servername, $username, $password, $dbname);
        // Check connection
        if ($conn->connect_error) {
            die("Connection failed: " . $conn->connect_error);
        } 
        $val = " (ten, Stt, nhietdo, doam, anhsang, timest)
        VALUES ('$ten', '$Stt', '$nhietdo', '$doam', '$anhsang', '$timest')";
        
        $sql = "INSERT INTO ";
        $sql .= $table;
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