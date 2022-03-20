#include <Arduino.h>
#include <HardwareSerial.h>
#include <string.h>
#include <ArduinoJson.h>
#define led1 PB13
#define led2 PB12

String data_rx = "";
String Protocol, Device, Stt, Temp, Humi, Light;
DynamicJsonDocument dataJson(1024);

HardwareSerial Blu_gate1(PA10, PA9);  // TX-PA10, RX-PA9
HardwareSerial Blu_gate2(PB11, PB10); // TX-PB11, RX-PB9
HardwareSerial Wif_gate(PA3, PA2); 
void getJson(String str);
void ClearVal();
float val(String str);

void setup() {
  // put your setup code here, to run once:
  Blu_gate1.begin(115200);
  Blu_gate2.begin(115200);
  Wif_gate.begin(115200); 
  pinMode(led1, OUTPUT);
  pinMode(led2,OUTPUT);
  digitalWrite(led1, HIGH);
  digitalWrite(led2, HIGH);
}

void loop() {
  // put your main code here, to run repeatedly:
  if(Blu_gate1.available()>0)
  {
    Protocol = "BLU";
    digitalWrite(led1, LOW);
    data_rx = Blu_gate1.readStringUntil('\n');
    getJson(data_rx);
    String x = Blu_gate1.readString();
    delay(500);
    digitalWrite(led1, HIGH);
  }

  if(Blu_gate2.available()>0)
  {
    Protocol = "BLU";
    digitalWrite(led2,LOW);
    data_rx = Blu_gate2.readStringUntil('\n');
    getJson(data_rx);
    String x = Blu_gate2.readString();
    delay(500);
    digitalWrite(led2,HIGH);
  }
}

void getJson(String str)
{
  String x="";

  Device = str.substring(0,str.indexOf(','));
  str.remove(0,str.indexOf(',')+1);
  x = str.substring(0,str.indexOf(','));
  x.remove(0,x.indexOf(':')+1);
  Temp = val(x);

  str.remove(0,str.indexOf(',')+1);
  x = str.substring(0,str.indexOf(','));
  x.remove(0,x.indexOf(':')+1);
  Humi = val(x);

  str.remove(0,str.indexOf(',')+1);
  x = str.substring(0,str.indexOf(','));
  x.remove(0,x.indexOf(':')+1);
  Light = val(x);

  str.remove(0,str.indexOf(',')+1);
  x = str.substring(0,str.indexOf("\r"));
  Stt = str;

  x = Protocol + "," + Device + "," + Temp + "," + Humi + "," +Light + "," + Stt;
  dataJson["P"] = Protocol;
  dataJson["N"] = Device;
  dataJson["T"] = Temp;
  dataJson["H"] = Humi;
  dataJson["L"] = Light;
  dataJson["S"] = Stt;
  String msg ;
  serializeJson(dataJson, msg);
  if(Device !="")
  {
    Wif_gate.println(msg);
    Wif_gate.println(msg.length());
    Wif_gate.flush();
  }
  x = "";
  ClearVal();
}
  
void ClearVal()
{
  Protocol = "";
  Device = "";
  Stt = "";
  Temp = "";
  Humi = "";
  Light = "";
  data_rx="";
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