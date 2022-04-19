//************************************************************
// code test RTT node root theo khoang cach
//************************************************************

#include <Arduino.h>
#include <painlessMesh.h>
#include <WiFiClient.h>

#define MESH_PREFIX "tung"
#define MESH_PASSWORD "phuongtung0801"
#define MESH_PORT 5555

#define STATION_SSID "P401"
#define STATION_PASSWORD "cutieubao"

#define HOSTNAME "MQTT_Bridge"

const unsigned long eventInterval = 5000;
unsigned long previousTime = 0;
unsigned int count = 1;

// Prototypes
void receivedCallback(const uint32_t &from, const String &msg);

// config static IP
IPAddress local_IP(192, 168, 1, 8);
IPAddress gateway(192, 168, 1, 3);
IPAddress subnet(255, 255, 0, 0);
IPAddress primaryDNS(8, 8, 8, 8);
IPAddress secondaryDNS(8, 8, 4, 4);

int meshNodeID = 9;

painlessMesh mesh;
void newConnectionCallback(uint32_t nodeId);
void changedConnectionCallback();
void nodeTimeAdjustedCallback(int32_t offset);

void setup()
{
  Serial.begin(115200);

  mesh.setDebugMsgTypes(ERROR | STARTUP | CONNECTION); // set before init() so that you can see startup messages

  // Channel set to 6. Make sure to use the same channel for your mesh and for you other
  // network (STATION_SSID)
  mesh.init(MESH_PREFIX, MESH_PASSWORD, MESH_PORT, WIFI_AP_STA, 6);
  mesh.onReceive(&receivedCallback);
  mesh.onNewConnection(&newConnectionCallback);
  mesh.onChangedConnections(&changedConnectionCallback);
  mesh.onNodeTimeAdjusted(&nodeTimeAdjustedCallback);
  // mesh.stationManual(STATION_SSID, STATION_PASSWORD);
  // mesh.setHostname(HOSTNAME);

  WiFi.begin(STATION_SSID, STATION_PASSWORD);
  Serial.println("Mesh root node initialized");

  while (WiFi.status() != WL_CONNECTED)
  {
    delay(1000);
    Serial.println("Connecting to WiFi..");
  }
  Serial.println("Connected to the WiFi network");
  Serial.println(WiFi.localIP());
  // mqttClient.connect(clientID, "", "", "theEndTopic", 1, true, "offline");
}

void loop()
{
  mesh.update();
}

void receivedCallback(const uint32_t &from, const String &msg)
{
  Serial.printf("bridge: Received from %u msg= %s\n", from, msg.c_str());
  // String topic = "painlessMesh/from/2131961461";
  String topic = "painlessMesh/from/" + String(from);

  Serial.printf("Receive message successful: %d time", count);
  count++;

  Serial.println("Data received");
  mesh.sendSingle(from, "Response for node" + String(from));
}

void newConnectionCallback(uint32_t nodeId)
{
  Serial.printf("--> startHere: New Connection, nodeId = %u\n", nodeId);
}

void changedConnectionCallback()
{
  Serial.printf("Changed connections\n");
}

void nodeTimeAdjustedCallback(int32_t offset)
{
  Serial.printf("Adjusted time %u. Offset = %d\n", mesh.getNodeTime(), offset);
}

IPAddress getlocalIP()
{
  return IPAddress(mesh.getStationIP());
}
