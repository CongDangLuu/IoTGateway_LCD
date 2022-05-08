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
// const char* pathWriteData = "http://iotgatewaylvtn.online/file_iotgatway/WriteData.php";
// const char* pathWriteConnect = "http://iotgatewaylvtn.online/file_iotgatway/DVstt.php";
// const char* pathGetCtr = "http://iotgatewaylvtn.online/file_iotgatway/control/control.json";

const char* pathWriteData = "http://iotgatewaylvtn.online/file_iotgatway/WriteData.php";
const char* pathWriteConnect = "http://iotgatewaylvtn.online/file_iotgatway/DVstt.php";
const char* pathGetCtr = "http://iotgatewaylvtn.online/file_iotgatway/control/control.json";


//đọc dữ liệu uart

SoftwareSerial Serial_STM(D2,D3);//D2 = RX -- D3 = TX

/*-----------mesh network----------------*/
Scheduler userScheduler; 
painlessMesh mesh;
uint32_t WifiID1, WifiID2 = 0;
typedef struct{
  uint32_t id[5];
  int cnt[5]={0,0,0,0,0};
  int Pre_cnt[5]={0,0,0,0,0};
}Infor;

Infor Wifi_c, Sub_c, Blu_c;
void set_infor();





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

// int Wif1_c, Wif2_c, Blu1_c, Blu2_c, Sub_c = 0; //biến kết nối cổng
// int PreWif1_c, PreWif2_c, PreBlu1_c, PreBlu2_c, PreSub_c = 0; 

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
// void rang()
// {
//   Wifi_c.cnt[0]= random(2);
//   Wifi_c.cnt[1]= random(2);
//   Blu_c.cnt[0]= random(2);
//   Blu_c.cnt[1]= random(2);
//   Sub_c.cnt[0] = random(2);
// }

// void testdatabase(){
//   Device ="DHTNode";
//   Temp = 20.00 + random(1000)/100;
//   Humi = 50.00 + random(1000)/100;
//   Light = 70.00 + random(1000)/100;
//   int t = random(2);
//   String tt = "OFF";
//   if (t)
//   {
//     tt = "ON";
//   }
//   Stt = tt;
//   Protocol = "WIFI1";
//   DB_post();
//   delay(10000);
// }
/*-----------------test--------------------*/


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
  set_infor();
  

  
  
}

void loop() {
  GetCtr();
  mesh.update();
  Read_Stm();
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
    for(int i = 0; i<5;i++){
      if(from == Wifi_c.id[i]){//Kiểm tra cổng nhận dữ liệu.
        Protocol = "WIFI" + String(i+1);
        break;
      }
    }
    
    Serial.println("--------------------------------------------------------------------------------");
    Serial.printf("Mesh Network: Received from %s: %u msg=%s\n",Protocol.c_str(), from, msg.c_str());
    readDataJson(JsonWifi);
    
    DB_post();
  }
  
}
void newConnectionCallback(uint32_t nodeId) {
}
void changedConnectionCallback() {
  Serial.println("---------------------------------");
  SimpleList<uint32_t> nodelist;
  
  nodelist = mesh.getNodeList();
  int a =nodelist.size();
  SimpleList<uint32_t>::iterator node = nodelist.begin();
  
  Serial.printf("Num nodes: %d\n",a);
 
  for(int i = 0;i<5;i++){
    while(node != nodelist.end()){
      if(Wifi_c.id[i]!= *node){
        Wifi_c.cnt[i] =1;
        break;
      }
      else{
        Wifi_c.cnt[i] =0;
      }
    }
  }

  postSttConncect();
  Serial.println(" WifiID0: "+ String(Wifi_c.id[0]) +"---" + String(Wifi_c.cnt[0]));
  Serial.println(" WifiID1: "+ String(Wifi_c.id[1]) +"---" + String(Wifi_c.cnt[1]));
  Serial.println(" WifiID2: "+ String(Wifi_c.id[2]) +"---" + String(Wifi_c.cnt[2]));
  Serial.println(" WifiID3: "+ String(Wifi_c.id[3]) +"---" + String(Wifi_c.cnt[3]));
  Serial.println(" WifiID4: "+ String(Wifi_c.id[4]) +"---" + String(Wifi_c.cnt[4]));
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
  String DVCtr,DVStt,MCU ="";

  DynamicJsonDocument cmdJson(1024);
  DeserializationError error = deserializeJson(cmdJson, payload);  
  if (error)
  {
    Serial.print("deserializeJson() failed: ");
    Serial.println(error.c_str());
  }
  else
  {
    //Protocol, Stt, MCU
    const char* name = cmdJson["Pro"];
    DVCtr = String(name);
    const char* TT = cmdJson["Stt"];
    DVStt = String(TT);
    const char* vdk = cmdJson["MCU"];
    MCU = String(vdk);
    if(MCU=="ESP"){
      for (int i = 0; i < 5; i++)
      {
        String gate = "Wifi"+ String(i+1);
        if(DVCtr == gate)
        {
          DynamicJsonDocument doc(1024);
          doc["cmd"] = "control";
          doc["Ctr"] = DVStt;
          String msg;
          serializeJson(doc, msg);
          mesh.sendSingle(Wifi_c.id[i],msg);
        }
      }
    }    
    else if(MCU=="STM") {
      String msgControl = DVCtr +","+DVStt;
      Serial_STM.println(msgControl);
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
    //data_rx = "connect,1,1,0,0,0,0,0";
    data_rx = Serial_STM.readStringUntil('\n');
    Serial.println(data_rx);
    
    String x="";
    x = data_rx.substring(0,data_rx.indexOf(','));
    if(x=="connect"){
      readConnectJson(data_rx);
    }
    else{
      data_rx.remove(0,data_rx.indexOf(',')+1);
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
    Stt = str;
    
    
    DB_post();
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
  //"connect,1,1,0,0,0,0,0"
    String x = "";
    for(int i =0; i<2;i++){
      str.remove(0,str.indexOf(',')+1);
      x = str.substring(0,str.indexOf(','));
      Blu_c.cnt[i] = x.toInt(); 
    }
    for(int a=0;a<5;a++){
      str.remove(0,str.indexOf(',')+1);
      x = str.substring(0,str.indexOf(','));
      Sub_c.cnt[a] = x.toInt();
    }
   
    postSttConncect();
  
}
void postSttConncect(){
  for(int i=0;i<5;i++){
    if((Wifi_c.cnt[i]!=Wifi_c.Pre_cnt[i])||(Blu_c.cnt[i]!=Blu_c.Pre_cnt[i])||(Sub_c.cnt[i]!=Sub_c.Pre_cnt[i])){
        if(WiFi.status()== WL_CONNECTED){
          //-------------------------------------------to send data to the database
          String  postData;
          String postWifi_c, postBlu_c, postSub_c ="";
          for (int i = 0; i < 5; i++)
          {
            postWifi_c += "&WIFI" + String(i+1) +"="+ String(Wifi_c.cnt[i]);
            postSub_c  += "&SUB" + String(i+1) +"="+ String(Sub_c.cnt[i]);

            Wifi_c.Pre_cnt[i] = Wifi_c.cnt[i];
            Blu_c.Pre_cnt[i] = Blu_c.cnt[i];  
            Sub_c.Pre_cnt[i] = Sub_c.cnt[i];
          }
          postBlu_c = "&BLU1=" + String(Blu_c.cnt[0]) + "&BLU2=" + String(Blu_c.cnt[1]);
          postData = postWifi_c + postBlu_c + postSub_c;
   
          http.begin(client,pathWriteConnect);            //Specify request destination
          http.addHeader("Content-Type", "application/x-www-form-urlencoded");                  //Specify content-type header
        
          int httpCode = http.POST(postData);   //Send the request
          String payload = http.getString();    //Get the response payload
          //-------------------------------------------
          if (httpCode == 200) 
          { 
            Serial.println("Status connect gate uploaded successfully."); 
            // Serial.println(httpCode); 
            // String webpage = http.getString();    // Get html webpage output and store it in a string
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
        break;
    }
  }
 
    
  
  
}

void set_infor(){ 
  Wifi_c.id[0] = 2486190338;
  Wifi_c.id[1] = 2487404895;
  Wifi_c.id[2] = 0;
  Wifi_c.id[3] = 0;
  Wifi_c.id[4] = 0;
 
  Sub_c.id[0] = 27918;
  Sub_c.id[1] = 89184;
  Sub_c.id[2] = 99632;
  Sub_c.id[3] = 75729;
  Sub_c.id[4] = 42593;
    
}
