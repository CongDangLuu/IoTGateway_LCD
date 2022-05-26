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
#define ssid "Hoang Yen"
#define pass "VNPT123@@2021"
// #define ssid "LCD"
// #define pass "244466666"

//database


const char* pathWriteData = "http://luanvanlogistic.highallnight.com/AppIoTgateway/WriteData.php";
const char* pathWriteConnect = "http://luanvanlogistic.highallnight.com/AppIoTgateway/DVconnect.php";
const char* pathGetCtr = "http://luanvanlogistic.highallnight.com/AppIoTgateway/control/control.json";


//đọc dữ liệu uart

SoftwareSerial Serial_STM(D5,D6);//D2 = RX -- D3 = TX

/*-----------mesh network----------------*/
Scheduler userScheduler; 
painlessMesh mesh;


typedef struct{
  uint32_t id[5];
  int cnt[5]={0,0,0,0,0};
  int Pre_cnt[5]={0,0,0,0,0};
  int ledpin[5];
}Infor;

Infor Wifi_c, Sub_c, Blu_c;
void set_infor();


void sendMessage(); 
Task taskSendMessage( TASK_SECOND * 1 , TASK_FOREVER, &sendMessage );

int NumNode=0;
void receivedCallback(const uint32_t &from, const String &msg);
void newConnectionCallback(uint32_t nodeId);
void changedConnectionCallback();
void droppedConnectionCallback(uint32_t nodeId);
void nodeTimeAdjustedCallback(int32_t offset);
void mesh_setup();
void Wifi_connect();
void Wifi_reconnect();
/*----------------------------------------*/


/*------Post dữ liệu lên database------------*/
HTTPClient http;                                                              //Declare object of class HTTPClient
WiFiClient client;

String apiKeyValue = "iotgateway2021";                                        //ket kết nối php
String Protocol, Device, Stt, Temp, Humi, Light ="";

String WifiPreData[5];




//post dữ liệu

int platPostData = 0;
void readDataJson(DynamicJsonDocument doc, int index);
void DB_post();                                                               //ghi dữ liệu vào database
//post connect


/*-------------------------------------------*/

/*-------------Điều khiển từ app---------*/
int skip = 0;
String  payload,Pre_payload="";

void GetCtr();                                                              // đọc Json điều khiển từ file control.json
void CtrCMD(String payload);                                                //xử l  ý tín hiệu điều khiển
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



void setup() {
  // put your setup code here, to run once:
  pinMode(Led_OnBoard, OUTPUT);
  pinMode(D5,INPUT);//RX
  pinMode(D6,OUTPUT);//TX
  Serial.begin(115200);
  Serial_STM.begin(115200);
  Serial.println();
  Serial.println("--------------------------------------------------------");
  Serial.println("----------------------Start here!-----------------------");
  
  mesh_setup();
  set_infor();
  Wifi_connect();
  
  

  
  
}

void loop() {
  GetCtr();
  userScheduler.execute(); 
  mesh.update();
  Read_Stm();

  Wifi_reconnect();
  
}

void Wifi_connect()
{
  delay(200);
  WiFi.begin(ssid, pass);     //Connect to your WiFi router
  Serial.println("");

  Serial.print("Connecting");
  //Wait for connection
  while (WiFi.status() != WL_CONNECTED) 
  {
    Serial.print(".");
    Blink_led();
  }
  Serial.println("");
  Serial.println("Wifi connected successfull!!!");
  delay(1000);
  Serial_STM.println("Start");
  digitalWrite(Led_OnBoard, HIGH);
}

void Wifi_reconnect(){
  if(WiFi.status() != WL_CONNECTED){
    Wifi_connect();
  }
}

void mesh_setup()
{
  mesh.setDebugMsgTypes(ERROR | STARTUP | CONNECTION); // set before init() so that you can see startup messages
  // Channel set to 6. Make sure to use the same channel for your mesh and for you other
  // network (STATION_SSID)
  mesh.init( MESH_PREFIX, MESH_PASSWORD, &userScheduler, MESH_PORT );
  mesh.stationManual(ssid, pass);
  mesh.setHostname(HOSTNAME);
  mesh.onNewConnection(&newConnectionCallback);
  mesh.onChangedConnections(&changedConnectionCallback);
  mesh.onNodeTimeAdjusted(&nodeTimeAdjustedCallback);
  mesh.onDroppedConnection([](auto nodeId) {});
  mesh.onReceive(&receivedCallback);
  userScheduler.addTask(taskSendMessage);
  taskSendMessage.enable();
  
}

void sendMessage()
{
  if(NumNode>0){
    Serial.println("____________");
    Serial.println("send request");
    DynamicJsonDocument doc(1024);
    String msg;
    doc["cmd"] = "request";
    serializeJson(doc, msg);
    mesh.sendBroadcast( msg );
    taskSendMessage.setInterval(TASK_SECOND * 60); 
  }
  
}
void receivedCallback(const uint32_t &from, const String &msg)
{
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
        Serial.println("--------------------------------------------------------------------------------");
        Serial.printf("Mesh Network: Received from %s: %u msg=%s\n",Protocol.c_str(), from, msg.c_str());
        readDataJson(JsonWifi, i);
        break;
      }
    }
  } 
}
void newConnectionCallback(uint32_t nodeId) {
}

void changedConnectionCallback() {
  Serial.println("---------------------------------");
    for (int i = 0; i < 5; i++){
      SimpleList<uint32_t> nodelist;
      nodelist = mesh.getNodeList();
      NumNode = nodelist.size();
      SimpleList<uint32_t>::iterator node = nodelist.begin();
      while(node != nodelist.end()){
        if(Wifi_c.id[i] == *node){
          Wifi_c.cnt[i] = 1;
          break;
        }
        else{
          Wifi_c.cnt[i] = 0;
        }
        node++;
      }
  }
  postSttConncect();

  Serial.println("---------------------------------");
}

void nodeTimeAdjustedCallback(int32_t offset) {   
}

void readDataJson(DynamicJsonDocument doc, int index){
  //{"N":"DHTNode","T":"00.00","H":"00.00","L":"00.00","S":"ON"}
    const char* name = doc["N"];
    Device = String(name);
    const char* TT = doc["S"];
    Stt = String(TT);
    float t = doc["T"];
    Temp = String(t);
    float h = doc["H"];
    Humi = String(h);
    float l = doc["L"];
    Light = String(l);

    String x = Protocol + "," + Device + "," + String(Temp) + "," + String(Humi) + "," +String(Light) + "," + Stt;
    if(x!=WifiPreData[index]){
      DB_post();
      WifiPreData[index] = x;
    }
    else{
      Serial.println("data not change");
      Serial.println("--------------------------------------------------------------------------------");
    }
     
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
     
      
      if (httpCode == 200) 
      { 
        Serial.print("Data uploaded successfully.   "); 
        Serial.println(httpCode); 
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
    const char* name = cmdJson["Device"];
    DVCtr = String(name);
    const char* TT = cmdJson["Stt"];
    DVStt = String(TT);
    const char* vdk = cmdJson["MCU"];
    MCU = String(vdk);
    if(MCU=="ESP"){
      for (int i = 0; i < 5; i++)
      {
        String gate = "WIFI"+ String(i+1);
        if(DVCtr == gate)
        {
          DynamicJsonDocument doc(1024);
          doc["cmd"] = "control";
          doc["Stt"] = DVStt;
          String msg;
          serializeJson(doc, msg);
          mesh.sendSingle(Wifi_c.id[i],msg);
          Serial.println(msg);
          break;
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
  Temp = "" ;
  Humi = "";
  Light = "" ;
  Stt = "";
}
void Blink_led()
{ 
  stt_led = !stt_led;
  digitalWrite(Led_OnBoard, stt_led);
  delay(250);
}

void Read_Stm(){
  if(Serial_STM.available()){
    //data_rx = "SUB1,SubNode1,32.60,72.00,0.00,OFF";
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
    Temp = x;
    //Temp = roundf(x.toFloat());

    str.remove(0,str.indexOf(',')+1);
    x = str.substring(0,str.indexOf(','));
    Humi = x;
    
    str.remove(0,str.indexOf(',')+1);
    x = str.substring(0,str.indexOf(','));
    Light = x;
    
    str.remove(0,str.indexOf(',')+1);
    x = str.substring(0,str.indexOf("\r")-4);
    Stt = str; 
    DB_post();
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
          for (int j = 0; j < 5; j++)
          {
            postWifi_c += "&WIFI" + String(j+1) +"="+ String(Wifi_c.cnt[j]);
            postSub_c  += "&SUB" + String(j+1) +"="+ String(Sub_c.cnt[j]);

            Wifi_c.Pre_cnt[j] = Wifi_c.cnt[j];
            Blu_c.Pre_cnt[j] = Blu_c.cnt[j];  
            Sub_c.Pre_cnt[j] = Sub_c.cnt[j];
            digitalWrite(Wifi_c.ledpin[j], !(Wifi_c.cnt[j]));

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
            Serial.println( postData);
            Serial.println("Status connect gate uploaded successfully."); 
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
  Wifi_c.id[0] = 2741624898;
  Wifi_c.id[1] = 1415703260 ;
  Wifi_c.id[2] = 2486190338;
  Wifi_c.id[3] = 4208757000;
  Wifi_c.id[4] = 2741294106;
 
  Sub_c.id[0] = 27918;
  Sub_c.id[1] = 89184;
  Sub_c.id[2] = 99632;
  Sub_c.id[3] = 75729;
  Sub_c.id[4] = 42593;

  Wifi_c.ledpin[0] = D0;
  Wifi_c.ledpin[1] = D1;
  Wifi_c.ledpin[2] = D2;
  Wifi_c.ledpin[3] = D3;
  Wifi_c.ledpin[4] = D4;
  for (int i = 0; i < 5; i++)
  {
    pinMode(Wifi_c.ledpin[i], OUTPUT);  
    digitalWrite(Wifi_c.ledpin[i], HIGH);
  }
    
}