#include <Arduino.h>
#include <SoftwareSerial.h>
#include <painlessMesh.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>
#include <string.h>


#define Led_OnBoard 2

//mesh network
#define MESH_PREFIX     "IoTGateway"
#define MESH_PASSWORD   "IoTGateway2021"
#define MESH_PORT   5555
#define HOSTNAME "MeshNetwork" 

//wifi
#define ssid "Hoang Yen"
#define pass "VNPT123@@2021"

//database

const char* serverName ="http://iotgatwae.000webhostapp.com/WriteData.php";
const char* pathGetCtr = "http://iotgatwae.000webhostapp.com/control/control.json";

byte stt_led = LOW;

String apiKeyValue = "iotgateway2021";//ket kết nối php

String Protocol, Device, Stt, Table;
String  payload,Pre_payload="";

float Temp, Humi, Light_Lux;

String data_rx="";//đọc dữ liệu từ stm
SoftwareSerial Serial_STM(D2,D3);//D2 = RX -- D3 = TX

Scheduler userScheduler; 
painlessMesh mesh;
uint32_t nodeID1, nodeID2 = 0;

HTTPClient http;    //Declare object of class HTTPClient
WiFiClient client;


void receivedCallback(const uint32_t &from, const String &msg);
void newConnectionCallback(uint32_t nodeId);
void changedConnectionCallback();
void nodeTimeAdjustedCallback(int32_t offset);
void mesh_setup();
void Wifi_connect();

void Blink_led();
void ClearVal();

//Post dữ liệu lên dâtbase
void DB_post();
void Json(String str);
void Data_uart(String str);

//Điều khiển từ app
void GetCtr();
void CtrCMD(String payload);

void test()
{
  Protocol = "WIFI2";
    
    Device = "abc";
    
    Stt = "on";
    Temp = 23.32;
    Humi =56.65;
    Light_Lux = 78.87;
    String x = Protocol + "," + Device + "," + String(Temp) + "," + String(Humi) + "," +String(Light_Lux) + "," + Stt;
    Serial.println("data php : " +x);

    DB_post();
    delay(1000);
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
  test();
  mesh.update();
  if(Serial_STM.available()>0)
  { 
    data_rx = Serial_STM.readString();
    Serial.println("Data from STM: " + data_rx);
    Data_uart(data_rx);   
  }
  delay(3000);
  
}


void receivedCallback(const uint32_t &from, const String &msg)
{
  Serial.printf("Mesh Network: Received from %u msg=%s\n", from, msg.c_str());
  String json = msg.c_str();
  DynamicJsonDocument doc(1024);
  DeserializationError error = deserializeJson(doc, json);
  if (error)
  {
    Serial.print("deserializeJson() failed: ");
    Serial.println(error.c_str());
  }
  else
  {
    Protocol = "WIF";
    const char* name = doc["Device"];
    Device = String(name);
    const char* TT = doc["Stt"];
    Stt = String(TT);
    Temp = doc["Temp"];
    Humi =doc["Humi"];
    Light_Lux = doc["Light"];
    String x = Protocol + "," + Device + "," + String(Temp) + "," + String(Humi) + "," +String(Light_Lux) + "," + Stt;
    Serial.println("data php : " +x);
    DB_post();
  }
  
}
void newConnectionCallback(uint32_t nodeId) {
    Serial.printf("--> startHere: New Connection, nodeId = %u\n", nodeId);
    if(nodeID1 == 0){
      nodeID1 = nodeId;
    }
    else
    {
      nodeID2 = nodeId;
    }

}
void changedConnectionCallback() {
  Serial.printf("Changed connections\n");
  Serial.printf("MeshReliability: Changed connections %s\n", mesh.subConnectionJson().c_str());
}
void nodeTimeAdjustedCallback(int32_t offset) {
    Serial.printf("Adjusted time %u. Offset = %d\n", mesh.getNodeTime(),offset);
}
void mesh_setup()
{
  mesh.setDebugMsgTypes(ERROR | STARTUP | CONNECTION); // set before init() so that you can see startup messages
  // Channel set to 6. Make sure to use the same channel for your mesh and for you other
  // network (STATION_SSID)
  mesh.init(MESH_PREFIX, MESH_PASSWORD, MESH_PORT, WIFI_AP_STA, 6);
  mesh.onReceive(&receivedCallback);
  mesh.stationManual(ssid, pass);
  mesh.setHostname(HOSTNAME);
  mesh.onNewConnection(&newConnectionCallback);
  mesh.onChangedConnections(&changedConnectionCallback);
  mesh.onNodeTimeAdjusted(&nodeTimeAdjustedCallback);
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
void Blink_led()
{ 
  stt_led = !stt_led;
  digitalWrite(Led_OnBoard, stt_led);
  delay(250);
}

void DB_post()
{
  if(Device != "")
  {
      if(WiFi.status()== WL_CONNECTED)
    {
      
      //-------------------------------------------to send data to the database
      String  postData;
      postData ="api_key=" + apiKeyValue + "&ten=" + Device + "&Stt=" + Stt + "&nhietdo=" + Temp + "&doam=" + Humi + "&anhsang=" + Light_Lux + "&table="+ Protocol;
            
      http.begin(client,serverName);            //Specify request destination
      http.addHeader("Content-Type", "application/x-www-form-urlencoded");                  //Specify content-type header
    
      int httpCode = http.POST(postData);   //Send the request
      String payload = http.getString();    //Get the response payload
      //-------------------------------------------
      if (httpCode == 200) 
      { 
        Serial.println("Values uploaded successfully."); 
        Serial.println(httpCode); 
        String webpage = http.getString();    // Get html webpage output and store it in a string
        Serial.println(webpage + "\n"); 
       Serial.println(postData);
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

void ClearVal()
{
  Protocol = "";
  Device = "";
  Temp = 0 ;
  Humi = 0;
  Light_Lux = 0 ;
  Stt ="";
}

void Data_uart(String str)
{ 
  String x= "";
  Protocol = str.substring(0,str.indexOf(','));
  str.remove(0,str.indexOf(',')+1);
  Device = str.substring(0,str.indexOf(','));
  str.remove(0,str.indexOf(',')+1);
  x = str.substring(0,str.indexOf(','));
  Temp = x.toFloat();

  str.remove(0,str.indexOf(',')+1);
  x = str.substring(0,str.indexOf(','));
  Humi = x.toFloat();

  str.remove(0,str.indexOf(',')+1);
  x = str.substring(0,str.indexOf(','));
  Light_Lux = x.toFloat();

  str.remove(0,str.indexOf(',')+1);
  x = str.substring(0,str.indexOf("\r"));
  if(x== "1"){
    Stt ="ON";
  }
  else if (x == "0")
  {
    Stt = "OFF";
  }
  else{Stt = x;}
  x = Protocol + "," + Device + "," + String(Temp) + "," + String(Humi) + "," +String(Light_Lux) + "," + Stt;
  if(Device !="")
  {
    Serial.println("data php : " +x);
    DB_post();
    
  }

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
        Serial.println(payload);    //Print request response payload-chuoi json
        Pre_payload = payload;
        CtrCMD(payload);
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
  }   

  if (DVCtr == "WIF1") 
  {
    mesh.sendSingle(nodeID1, DVStt);
  }
  else if(DVCtr == "WIF2")
  {
    mesh.sendSingle(nodeID2, DVStt);
  }else if( (DVCtr == "BLU1") || (DVCtr == "BLU2") || (DVCtr =="SUB"))
  {
    Serial_STM.println(payload);
  }

  
}