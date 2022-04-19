#include <Arduino.h>
// Node 1
#include "painlessMesh.h"

#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <DHT_U.h>


// WiFi Credentials
#define MESH_PREFIX     "IoTGateway"
#define MESH_PASSWORD   "IoTGateway2021"
#define MESH_PORT   5555

//Pin Declaration
#define Led_OnBoard 2

#define DHTPIN D3    // Digital pin connected to the DHT sensor 

#define DHTTYPE    DHT11     // DHT 11


Scheduler userScheduler; 
painlessMesh  mesh;


DHT_Unified dht(DHTPIN, DHTTYPE);

int LState, PreLState = 0;
uint32_t delayMS;
float Temp, Humi, PreTemp, PreHumi =0;



void sendMessage(); 
Task taskSendMessage( TASK_SECOND * 1 , TASK_FOREVER, &sendMessage );
void receivedCallback( uint32_t from, String &msg );
void mesh_setup();
void DHT11_setup();
// Needed for painless library


void newConnectionCallback(uint32_t nodeId) {
  //Serial.printf("--> startHere: New Connection, nodeId = %u\n", nodeId);
}
void changedConnectionCallback() {
  //Serial.printf("Changed connections\n");
}
void nodeTimeAdjustedCallback(int32_t offset) {
  //Serial.printf("Adjusted time %u. Offset = %d\n", mesh.getNodeTime(), offset);
}

void setup() {
  Serial.begin(115200);
  pinMode(Led_OnBoard, OUTPUT);
  digitalWrite(Led_OnBoard,(!LState));
  DHT11_setup();
  mesh_setup();
  
}
void loop() {
  userScheduler.execute();
  mesh.update();
  
}

void DHT11_setup(){
  dht.begin();
  sensor_t sensor;
  dht.temperature().getSensor(&sensor);
  // Print humidity sensor details.
  dht.humidity().getSensor(&sensor);
  delayMS = sensor.min_delay / 1000;
  Serial.println(delayMS);
}
void mesh_setup(){
  mesh.setDebugMsgTypes( ERROR | STARTUP );  
  mesh.init( MESH_PREFIX, MESH_PASSWORD, &userScheduler, MESH_PORT );
  mesh.onReceive(&receivedCallback);
  mesh.onNewConnection(&newConnectionCallback);
  mesh.onChangedConnections(&changedConnectionCallback);
  mesh.onNodeTimeAdjusted(&nodeTimeAdjustedCallback);
  userScheduler.addTask( taskSendMessage );
  taskSendMessage.enable();
}

void DHT11_data(){
  // Get temperature event and print its value.
  sensors_event_t event;
  dht.temperature().getEvent(&event);
  if (isnan(event.temperature)) {
    Serial.println(F("Error reading temperature!"));
  }
  else {
    Temp = event.temperature;
  }
  // Get humidity event and print its value.
  dht.humidity().getEvent(&event);
  if (isnan(event.relative_humidity)) {
    Serial.println(F("Error reading humidity!"));
  }
  else {
    Humi = event.relative_humidity;
  }
}
void sendMessage()
{
  DHT11_data();
  Serial.println("____________");
  
  // Serializing in JSON Format
  if((Temp!=PreTemp) || (Humi!=PreHumi) || (LState!= PreLState))
  {
    DynamicJsonDocument doc(1024);
    String stt = "OFF";
    String msg ;
    if(LState){
      stt = "ON";
    }
    doc["N"] = "DHTNode";
    doc["T"] = String(Temp);
    doc["H"] = String(Humi);
  // doc["T"] = String(32.23);
  // doc["H"] = String(75.57);
    doc["S"] = stt;

    serializeJson(doc, msg);
    mesh.sendBroadcast( msg );
    Serial.println("Message from DHT node:");
    Serial.println(msg);
    PreHumi = Humi;
    PreLState = LState;
    PreTemp =Temp;

  }
 taskSendMessage.setInterval(TASK_SECOND * 10); 
}
void receivedCallback( uint32_t from, String &msg ) {
  Serial.println("WiFi signal: " + String(WiFi.RSSI()) + " db");
  Serial.println();
  Serial.print("Message control form app= ");Serial.println(msg);
  String json;
  DynamicJsonDocument doc(1024);
  json = msg.c_str();
  DeserializationError error = deserializeJson(doc, json);
  if (error)
  {
    Serial.print("deserializeJson() failed: ");
    Serial.println(error.c_str());
  }
  LState = int(doc["Stt"]); 
  int a = !LState;
  digitalWrite(Led_OnBoard, a);
  sendMessage();
}




