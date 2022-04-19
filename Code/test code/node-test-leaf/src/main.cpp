//************************************************************
// code test RTT node leaf theo khoang cach
//************************************************************

#include <Arduino.h>
#include <painlessMesh.h>
#include <WiFi.h>

#define MESH_PREFIX "tung"
#define MESH_PASSWORD "phuongtung0801"
#define MESH_PORT 5555

#define STATION_SSID "TUNG"
#define STATION_PASSWORD "123456789"

#define HOSTNAME "MQTT_Bridge"

const unsigned long eventInterval = 5000;
size_t logServerId = 0;
size_t test = 0;

// Prototypes
void receivedCallback(const uint32_t &from, const String &msg);

// config static IP
IPAddress local_IP(192, 168, 1, 8);
IPAddress gateway(192, 168, 1, 3);
IPAddress subnet(255, 255, 0, 0);
IPAddress primaryDNS(8, 8, 8, 8);
IPAddress secondaryDNS(8, 8, 4, 4);

int meshNodeID = 9;
unsigned long previousTime = 0;
unsigned long currentTime = 0;
painlessMesh mesh;
Scheduler userScheduler; // to control your personal task

// Send message to the logServer every 10 seconds
Task myLoggingTask(5000, TASK_FOREVER, []()
                   {
                     previousTime = millis();
  if (logServerId == 0) // If we don't know the logServer yet
    mesh.sendBroadcast("ping from node leaf ");
  else
    mesh.sendSingle(logServerId, "ping from node leaf"); });

void setup()
{
  Serial.begin(9600);

  mesh.setDebugMsgTypes(ERROR | STARTUP | CONNECTION); // set before init() so that you can see startup messages

  // Channel set to 6. Make sure to use the same channel for your mesh and for you other
  // network (STATION_SSID)
  mesh.init(MESH_PREFIX, MESH_PASSWORD, MESH_PORT, WIFI_AP_STA, 6);
  mesh.onReceive(&receivedCallback);
  userScheduler.addTask(myLoggingTask);
  myLoggingTask.enable();

  WiFi.begin(STATION_SSID, STATION_PASSWORD);

  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.println("Connecting to WiFi..");
  }
  Serial.println("Connected to the WiFi network");
  // mqttClient.connect(clientID, "", "", "theEndTopic", 1, true, "offline");
}

void loop()
{
  userScheduler.execute(); // it will run mesh scheduler as well
  mesh.update();
}

void receivedCallback(const uint32_t &from, const String &msg)
{
  Serial.printf("bridge: Received from %u msg=%s\n", from, msg.c_str());
  // String topic = "painlessMesh/from/2131961461";
  String topic = "painlessMesh/from/" + String(from);
  Serial.println("Data received");
  currentTime = millis();
  Serial.print("RTT Time is: ");
  Serial.println(currentTime - previousTime);
}

IPAddress getlocalIP()
{
  return IPAddress(mesh.getStationIP());
}
// 1300337141

//     node2825211609