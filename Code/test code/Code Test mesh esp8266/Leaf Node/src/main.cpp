//************************************************************
// dht node to collect humid temp data in living room (living)
//************************************************************
#include "painlessMesh.h"
#include "DHT.h"
#include "math.h"

#define MESH_PREFIX "tung"
#define MESH_PASSWORD "phuongtung0801"
#define MESH_PORT 5555

#define DHTTYPE DHT11
#define DHTPin D3
#define LED D0
// const int DHTPin = 5;
DHT dht(DHTPin, DHTTYPE);

Scheduler userScheduler; // to control your personal task
painlessMesh mesh;

// Prototype
void receivedCallback(uint32_t from, String &msg);
void calculateRTT(unsigned int currentTime, unsigned int previousTime, unsigned int count);

size_t logServerId = 0;
size_t test = 0;
unsigned long previousTime = 0;
unsigned long currentTime = 0;
unsigned int count = 0;
unsigned int countRe = 0;
unsigned int countSend = 0;


Task myLoggingTask(5000, TASK_FOREVER, []()
                   {
                     previousTime = millis();
  // if (logServerId == 0) // If we don't know the logServer yet
    mesh.sendBroadcast("ping from node leaf ");
  // else
  //   mesh.sendSingle(logServerId, "ping from node leaf");
    Serial.println("task node leaf running"); });

void setup()
{
  Serial.begin(115200);
  dht.begin();
  delay(10);
  pinMode(D3, OUTPUT);
  pinMode(LED, OUTPUT);

  mesh.setDebugMsgTypes(ERROR | STARTUP | CONNECTION); // set before init() so that you can see startup messages

  mesh.init(MESH_PREFIX, MESH_PASSWORD, &userScheduler, MESH_PORT, WIFI_AP_STA, 6);
  mesh.onReceive(&receivedCallback);

  // Add the task to the your scheduler
  userScheduler.addTask(myLoggingTask);
  myLoggingTask.enable();
}

void loop()
{
  userScheduler.execute(); // it will run mesh scheduler as well
  mesh.update();
}

void receivedCallback(uint32_t from, String &msg)
{
  Serial.printf("logClient: Received from %u msg=%s\n", from, msg.c_str());
  // String topic = "painlessMesh/from/2131961461";
  String topic = "painlessMesh/from/" + String(from);
  Serial.println("Data received");
  currentTime = millis();
  // print RTT of each msg send and response
  Serial.printf("Send message successful: %d time\n", count);

  

  Serial.printf("RTT Time %d is: ", count);
  count++;
  Serial.println(currentTime - previousTime);

  calculateRTT(currentTime, previousTime, count);
  Serial.println();
  
}

// note la bien count chay tu 1
void calculateRTT(unsigned int currentTime, unsigned int previousTime, unsigned int count)
{
  Serial.println("calculateRTT");
  // khoi tao gia tri RTT vong dau tien
  int roundTripTime = currentTime - previousTime;
  // array de luu RTT, 300 phan tu gui trong 5ph
  static unsigned int arrayRTT[30];
  // RTT thu count luu vao mang
  arrayRTT[count] = roundTripTime;

  // gan gia tri dau tien cho max va min
  static unsigned int max = arrayRTT[1];
  static unsigned int min = arrayRTT[1];
  // for loop through array

  if (arrayRTT[count] - min > 42949672)
  {
    min = arrayRTT[count];
  }
  if (arrayRTT[count] >= max)
  {
    max = arrayRTT[count];
  }
  Serial.printf("min RTT is: %d\n", min);
  Serial.printf("max RTT is: %d\n", max);
  // tinh RTT trung binh
  double sumRTT = 0;
  for (size_t i = 0; i <= count; i++)
  {
    sumRTT += (double)arrayRTT[i];
  }
  double averageRTT = sumRTT / (double)(count);
  Serial.printf("Average RTT is: %f \n", averageRTT);
}