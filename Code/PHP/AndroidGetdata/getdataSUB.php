<?php   
$connect = mysqli_connect("localhost", "id18702154_iotgateway", "LuanVanTotNghiep@@2022", "id18702154_iotgateway_database"); 

$sql = "SELECT * FROM SUB";

$result = mysqli_query($connect, $sql);

$json_array = array();  


while($row = mysqli_fetch_array($result)){
	$ten=$row['NameDV'];
    $nhiet_do = $row['Temperature'];
    $do_am = $row['Humidity'];
    $anh_sang = $row['Light'];
    $trang_thai =$row['Status_dv'];
    $thoi_gian_doc = $row['Timestamp'];
    
    
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
