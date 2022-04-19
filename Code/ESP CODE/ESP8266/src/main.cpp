#include <Arduino.h>
#include <SoftwareSerial.h>
#include <painlessMesh.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>

#include <math.h>
#include <string.h>


#define Led_OnBoard 2

//mesh network
#define MESH_PREFIX     "IoTGateway"
#define MESH_PASSWORD   "IoTGateway2021"
#define MESH_PORT   5555
#define HOSTNAME "MeshNetwork" 

//wifi
// #define ssid "Hoang Yen"
// #define pass "VNPT123@@2021"
#define ssid "LCD"
#define pass "244466666"

//database
const char* pathWriteData = "http://iotgateway.000webhostapp.com/WriteData.php";
const char* pathWriteConnect = "http://iotgateway.000webhostapp.com/DVstt.php";
const char* pathGetCtr = "http://iotgateway.000webhostapp.com/control/control.json";


//đọc dữ liệu uart

SoftwareSerial Serial_STM(D2,D3);//D2 = RX -- D3 = TX

/*-----------mesh network----------------*/
Scheduler userScheduler; 
painlessMesh mesh;
uint32_t WifiID1, WifiID2 = 0;

void receivedCallback(const uint32_t &from, const String &msg);
void newConnectionCallback(uint32_t nodeId);
void changedConnectionCallback();
void droppedConnectionCallback(uint32_t nodeId);
void nodeTimeAdjustedCallback(int32_t offset);
void mesh_setup();
void Wifi_connect();
/*----------------------------------------*/


/*------Post dữ liệu lên database------------*/
HTTPClient http;                                                              //Declare object of class HTTPClient
WiFiClient client;

String apiKeyValue = "iotgateway2021";                                        //ket kết nối php
String Protocol, Device, Stt;
float Temp, Humi, Light;

int Wif1_c, Wif2_c, Blu1_c, Blu2_c, Sub_c = 0; //biến kết nối cổng
int PreWif1_c, PreWif2_c, PreBlu1_c, PreBlu2_c, PreSub_c = 0; 

//post dữ liệu
void readDataJson(DynamicJsonDocument json);
void DB_post();                                                               //ghi dữ liệu vào database
//post connect


/*-------------------------------------------*/

/*-------------Điều khiển từ app---------*/
int skip = 0;
String  payload,Pre_payload="";

void GetCtr();                                                              // đọc Json điều khiển từ file control.json
void CtrCMD(String payload);                                                //xử lý tín hiệu điều khiển
/*-------------------------------------------*/

/*-------------Đọc dữ liệu uart---------*/
String data_rx="";            //biến lưu dữ liệu từ stm
void Read_Stm();                                                              // đọc dữ liệu trong Json nhận từ stm32f1   
void Get_Data_Stm(String str);
float val(String str);                                                        //đọc Json từ stm32f1(trại thái kết nối hoặc dữ liệu)
                          
//post connect
void readConnectJson(String str);                        //đọc trạng thái kết nối từ stm32f1
void postSttConncect();                                  //ghi trạng thái kết nối vào file DV_stt.php                               
/*---------------------------------------*/

/*---------------------------------------*/
byte stt_led = LOW;

void Blink_led();
void ClearVal();
/*---------------------------------------*/
/*-----------------test--------------------*/
void rang()
{
  Wif1_c= random(2);
  Wif2_c= random(2);
  Blu1_c= random(2);
  Blu2_c= random(2);
  Sub_c = random(2);
}

void testdatabase(){
  Device ="DHTNode";
  Temp = 20.00 + random(1000)/100;
  Humi = 50.00 + random(1000)/100;
  Light = 70.00 + random(1000)/100;
  int t = random(2);
  String tt = "OFF";
  if (t)
  {
    tt = "ON";
  }
  Stt = tt;
  Protocol = "WIFI1";
  DB_post();
  delay(10000);
}



void setup() {
  // put your setup code here, to run once:
  pinMode(Led_OnBoard, OUTPUT);
  pinMode(D2,INPUT);//RX
  pinMode(D3,OUTPUT);//TX
  Serial.begin(115200);
  Serial_STM.begin(115200);
  Serial.println();
  Serial.println("--------------------------------------------------------");
  Serial.println("----------------------Start here!-----------------------");
  
  mesh_setup();
  Wifi_connect();
  
  
}

void loop() {
  GetCtr();
  mesh.update();
  //Read_Stm();
  postSttConncect();
 
}

void Wifi_connect()
{
  delay(1000);
  WiFi.begin(ssid, pass);     //Connect to your WiFi router
  Serial.println("");

  Serial.print("Connecting");
  //Wait for connection
  while (WiFi.status() != WL_CONNECTED) 
  {
    Serial.print(".");
    Blink_led();
  }
  digitalWrite(Led_OnBoard, LOW);

  Serial.println("");
  Serial.println("Wifi connected successfull!!!");
  WiFi.setAutoReconnect(true);
  WiFi.persistent(true);
  delay(1000);
}

void mesh_setup()
{
  mesh.setDebugMsgTypes(ERROR | STARTUP | CONNECTION); // set before init() so that you can see startup messages
  // Channel set to 6. Make sure to use the same channel for your mesh and for you other
  // network (STATION_SSID)
  mesh.init(MESH_PREFIX, MESH_PASSWORD, MESH_PORT, WIFI_AP_STA, 6);
  mesh.stationManual(ssid, pass);
  mesh.setHostname(HOSTNAME);
  mesh.onNewConnection(&newConnectionCallback);
  mesh.onChangedConnections(&changedConnectionCallback);
  mesh.onNodeTimeAdjusted(&nodeTimeAdjustedCallback);
  mesh.onDroppedConnection([](auto nodeId) {});
  mesh.onReceive(&receivedCallback);
}
void receivedCallback(const uint32_t &from, const String &msg)
{
  //-------------------------------------
  //đọc RSSI.
 //mesh.rss

  //-------------------------------------
  
  String json = msg.c_str();
  DynamicJsonDocument JsonWifi(1024);
  DeserializationError error = deserializeJson(JsonWifi, json);
  if (error)
  {
    Serial.print("deserializeJson() failed: ");
    Serial.println(error.c_str());
  }
  else
  {
    //Kiểm tra cổng nhận dữ liệu.
    if(from == WifiID1){
      Protocol = "WIFI1";
    }
    else if(from == WifiID2){
      Protocol = "WIFI2";
    }
    Serial.printf("Mesh Network: Received from %s: %u msg=%s\n",Protocol.c_str(), from, msg.c_str());
    readDataJson(JsonWifi);
    
    DB_post();
  }
  
}
void newConnectionCallback(uint32_t nodeId) {
  

}
void changedConnectionCallback() {
  Serial.println("---------------------------------");
  SimpleList<uint32_t> nodes;
  uint32_t ID1, ID2 = 0;

  nodes = mesh.getNodeList();
  int a =nodes.size();
  SimpleList<uint32_t>::iterator node = nodes.begin();
  
  Serial.printf("Num nodes: %d\n",a);
  switch (a){
    case 0:
      if(WifiID1!=0 && WifiID2 != 0)//2 node -> 1 node
      {
        break;
      }else{
        ID1 = ID2 = 0;
        WifiID1 = WifiID2 = 0;
        Wif1_c = Wif2_c = 0;
      }
      
      break;
    case 1:
      ID1 = *node;
      if(WifiID1==0 && WifiID2 == 0)//0 node -> 1 node
      {
        WifiID1 = ID1;
        Wif1_c = 1;
        Wif2_c = 0;
      }else if(WifiID1!=0 && WifiID2 != 0)//2 node -> 1 node
      {
        if(ID1 ==  WifiID1){
          WifiID2 = 0;
          Wif1_c = 1;
          Wif2_c = 0;
        }else if(ID1 ==  WifiID2){
          WifiID1 = 0;
          Wif1_c = 0;
          Wif2_c = 1;
        }
      }
      break;
    case 2:
      ID1 = *node;
      node++;
      ID2 = *node;
      WifiID1 = ID1;
      WifiID2 = ID2;
      Wif1_c = 1;
      Wif2_c = 1;
      
      break;

    default:
      break;
  }

  Serial.println(" WifiID1: "+ String(WifiID1));
  Serial.println(" WifiID2: "+ String(WifiID2));
  Serial.println("---------------------------------");
}

void nodeTimeAdjustedCallback(int32_t offset) {   
}

void readDataJson(DynamicJsonDocument doc){
  //{"N":"DHTNode","T":"00.00","H":"00.00","L":"00.00","S":"ON"}
    const char* name = doc["N"];
    Device = String(name);
    const char* TT = doc["S"];
    Stt = String(TT);
    Temp = doc["T"];
    Humi =doc["H"];
    Light = doc["L"];
    String x = Protocol + "," + Device + "," + String(Temp) + "," + String(Humi) + "," +String(Light) + "," + Stt;
    Serial.println("data php : " +x);
    
}

void DB_post()
{
  if(Device != "")
  {
      if(WiFi.status()== WL_CONNECTED)
    {
      
      String  postData;
      postData ="api_key=" + apiKeyValue + "&Name=" + Device + "&Stt=" + Stt + "&Temp=" + Temp + "&Humi=" + Humi + "&Light=" + Light + "&Table="+ Protocol;
            
      http.begin(client,pathWriteData);            //Specify request destination
      http.addHeader("Content-Type", "application/x-www-form-urlencoded");                  //Specify content-type header
    
      int httpCode = http.POST(postData);   //Send the request
      String payload = http.getString();    //Get the response payload
      
      if (httpCode == 200) 
      { 
        Serial.print("Data uploaded successfully.   "); 
        Serial.println(httpCode); 
        // String webpage = http.getString();    // Get html webpage output and store it in a string
        // Serial.println(webpage + "\n"); 
        // Serial.println(postData);
        Serial.println("--------------------------------------------------------------------------------");
      }
      else 
      { 
        Serial.println(httpCode); 
        Serial.println("Failed to upload values. \n"); 
        return; 
      }
      http.end();
    }else{Serial.println("Connect Wifi Error!!!");}
  }
  else{Serial.println("Protocol Error!!!");}
  ClearVal();
}

void GetCtr()
{
  http.begin(client, pathGetCtr);     //Specify request destination
  int httpCode = http.GET();            //Send the request
      //Get the response payload from server
  if(httpCode == 200){
    payload = http.getString();
    http.end();  //Close connection
      if(payload != Pre_payload)
      {
        Pre_payload = payload;
        if(skip){
          Serial.println(payload);    //Print request response payload-chuoi json
          CtrCMD(payload);
        }
        skip = 1;
        
      }
  }
}
void CtrCMD(String payload)
{
  String DVCtr,DVStt="";

  DynamicJsonDocument cmdJson(1024);
  DeserializationError error = deserializeJson(cmdJson, payload);  
  if (error)
  {
    Serial.print("deserializeJson() failed: ");
    Serial.println(error.c_str());
  }
  else
  {
    const char* name = cmdJson["Pro"];
    DVCtr = String(name);
    const char* TT = cmdJson["Stt"];
    DVStt = String(TT);
    if (DVCtr == "WIF1") {
      mesh.sendSingle(WifiID1, payload);
    }
    else if(DVCtr == "WIF2")  {
      mesh.sendSingle(WifiID2, payload);
    }
    else if( (DVCtr == "BLU1") || (DVCtr == "BLU2") || (DVCtr =="SUB")) {
      Serial_STM.println(payload);
    }
  }   

  

  
}

void ClearVal()
{
  Protocol = "";
  Device = "";
  Temp = 0 ;
  Humi = 0;
  Light = 0 ;
  Stt ="";
}
void Blink_led()
{ 
  stt_led = !stt_led;
  digitalWrite(Led_OnBoard, stt_led);
  delay(250);
}

void Read_Stm(){
  if(Serial_STM.available()){
    //data_rx = "SUB,DHTNode,32.60,72.00,0.00,OFF";
    //data_rx = "connect,1,1,1";
    data_rx = Serial_STM.readStringUntil('/n');
    
    String x="";
    x = data_rx.substring(0,data_rx.indexOf(','));
    data_rx.remove(0,data_rx.indexOf(',')+1);
    if(x=="connect"){
      readConnectJson(data_rx);
    }
    else{
      Protocol = x;
      Get_Data_Stm(data_rx);
    } 
  } 
}

void Get_Data_Stm(String str) {
    String x="";
    
    Device = str.substring(0,str.indexOf(','));
    
    str.remove(0,str.indexOf(',')+1);
    x = str.substring(0,str.indexOf(','));
    Temp = val(x);
    //Temp = roundf(x.toFloat());

    str.remove(0,str.indexOf(',')+1);
    x = str.substring(0,str.indexOf(','));
    Humi = val(x);
    
    str.remove(0,str.indexOf(',')+1);
    x = str.substring(0,str.indexOf(','));
    Light = val(x);
    
    str.remove(0,str.indexOf(',')+1);
    x = str.substring(0,str.indexOf("\r")-4);
    if(str=="0"){
      Stt = "OFF";
    }
    else if(str=="1"){
      Stt = "ON";
    }else{
      Stt = str;
    }
    
    DB_post();
    // Serial.println(Protocol);
    // Serial.println(Device);
    // Serial.println(Temp);
    // Serial.println(Humi);
    // Serial.println(Light);
    // Serial.println(Stt);
}
float val(String str){
  float a;
  if(str==""){
    a = 0;
  }
  else{
    a = str.toFloat();
  }
  return a;
}


void readConnectJson(String str){
  //"connect,1,1,1"
  String x = "";
    x = str.substring(0,str.indexOf(','));
    Blu1_c = x.toInt();
    
    str.remove(0,str.indexOf(',')+1);
    x = str.substring(0,str.indexOf(','));
    Blu2_c = x.toInt();
   
    str.remove(0,str.indexOf(',')+1);
    x = str.substring(0,str.indexOf("\r")-4);
    Sub_c = x.toInt();

    // Serial.println(Blu1_c);
    //  Serial.println(Blu2_c);
    // Serial.println(Sub_c);
    postSttConncect();
  
}
void postSttConncect(){
  if((Wif1_c != PreWif1_c) || (Wif2_c != PreWif2_c) || (Blu1_c!=PreBlu1_c) || (Blu2_c!=PreBlu2_c) || (Sub_c!= PreSub_c)){
    if(WiFi.status()== WL_CONNECTED)
    {
      
      //-------------------------------------------to send data to the database
      String  postData;
      postData ="&WIFI1=" + String(Wif1_c) + "&WIFI2=" + String(Wif2_c) + "&BLU1=" + String(Blu1_c) + "&BLU2=" + String(Blu2_c) + "&SUB=" + String(Sub_c);
            
      http.begin(client,pathWriteConnect);            //Specify request destination
      http.addHeader("Content-Type", "application/x-www-form-urlencoded");                  //Specify content-type header
    
      int httpCode = http.POST(postData);   //Send the request
      String payload = http.getString();    //Get the response payload
      //-------------------------------------------
      if (httpCode == 200) 
      { 
        Serial.println("Status connect gate uploaded successfully."); 
        Serial.println(httpCode); 
        String webpage = http.getString();    // Get html webpage output and store it in a string
        Serial.println("--------------------------------------------------------------------------------");
      }
      else 
      { 
        Serial.println(httpCode); 
        Serial.println("Failed to upload values. \n"); 
        return; 
      }
      http.end();
    }else{Serial.println("Connect Wifi Error!!!");}
   PreBlu1_c = Blu1_c;
   PreBlu2_c = Blu2_c;
   PreSub_c  = Sub_c;
   PreWif1_c = Wif1_c;
   PreWif2_c = Wif2_c;
  }
  
}

