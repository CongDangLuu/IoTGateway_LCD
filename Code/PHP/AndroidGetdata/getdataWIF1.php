<?php   
$connect = mysqli_connect("localhost", "id18138828_iotgateway", "Q]sac8*&Kq)3#M{c", "id18138828_gateway_database"); 

$sql = "SELECT * FROM WIFI1";

$result = mysqli_query($connect, $sql);

$json_array = array();  


while($row = mysqli_fetch_array($result)){
    $ten=$row['ten'];
    $nhiet_do = $row['nhietdo'];
    $do_am = $row['doam'];
    $anh_sang = $row['anhsang'];
    $trang_thai =$row['Stt'];
    $thoi_gian_doc = $row['timest'];
    
	$json_array[] = array(
	    'Name'=>$ten,
		'ND'=> $nhiet_do,
		'DA' => $do_am,
		'AS' => $anh_sang,
		'Time' => $thoi_gian_doc,
		'TT' => $trang_thai
	);
}
echo json_encode($json_array)
 ?>
