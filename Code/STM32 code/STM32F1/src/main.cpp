#include <Arduino.h>
#include <HardwareSerial.h>
#include <string.h>
#include <ArduinoJson.h>
#include <SPI.h>
#include <CC1101_RF.h>
#define led1 PB13
#define led2 PB12

HardwareSerial Blu_gate1(PA10, PA9);  // TX-PA10, RX-PA9
HardwareSerial Blu_gate2(PB11, PB10); // TX-PB11, RX-PB9
HardwareSerial Wif_gate(PA3, PA2); 

CC1101 radio;
byte CC1101_rx[64];

String Protocol, Device, Stt, Temp, Humi, Light = "";//biến lưu trữ dữ liệu

 typedef struct{
  String PreTemp ="";
  String PreHumi ="";
  String PreLight ="";
  String PreStt ="";
 } PreData;//Biến lưu dữ liệu trước đó.

  PreData PreBlu1;
  PreData PreBlu2;
  PreData PreSub;

String data_BluRx, data_SubRx = "";// biến nhận chuỗi dữ liệu từ các node

String data_control = "";// biến nhận chuỗi Json điều khiển từ esp.

int Blu1_c, Blu2_c, Sub_c = 0; 
int PreBlu1_c, PreBlu2_c, PreSub_c = 0; 

unsigned long timer;
unsigned long timer_Blu1_Now, timer_Blu2_Now, timer_Sub_Now = 0;

void getDataRx(String str, PreData gate);                                                                 // xử lý dữ liệu nhận được từ bluetooth và subghz
void DataJsonUART();    //gửi dữ liệu cho esp8266 dưới dạng Json

void getSttJson();
int getSttConnect(unsigned long timerNow);

void CC1101_TxData(String data);                                                            //Gửi tín hiệu điều khiển cho CC1101
void CC1101_RxData();                                                                       //nhận dữ liệu từ CC1101

void getCtr_Stm(String Ctr);                                                                //nhận tín hiệu điều khiển từ esp8266

void ClearVal();
float val(String str);
int CheckData(PreData gate);




void setup() {
  // put your setup code here, to run once:
  Blu_gate1.begin(115200);
  Blu_gate2.begin(115200);
  Wif_gate.begin(115200); 
  pinMode(led1, OUTPUT);
  pinMode(led2,OUTPUT);
  digitalWrite(led1, HIGH);
  digitalWrite(led2, HIGH);
  SPI.begin(); 
  radio.begin(433.2e6); 
  radio.setRXstate(); 
  //Wif_gate.println("Start............................");
  }

void loop() {
  
  //nhận lệnh điều khiển
  if(Wif_gate.available()>0)
  {
    data_control = Wif_gate.readStringUntil('\n'); //{"Pro":"BLU1","Stt":"off"}
    getCtr_Stm(data_control);
    String DeleteBuf = Wif_gate.readString();
  }
  
  // Cổng Blu1
  if(Blu_gate1.available()>0)
  {
    timer_Blu1_Now =  millis();
    Protocol = "BLU1";
    digitalWrite(led1, LOW);
    data_BluRx = Blu_gate1.readStringUntil('\n');
    Wif_gate.println(data_BluRx);
    // Blu_gate1.println(data_BluRx);
    getDataRx(data_BluRx,PreBlu1);
   
    String DeleteBuf = Blu_gate1.readString();
    digitalWrite(led1, HIGH);
  }
  
  //Cổng Blu2
  if(Blu_gate2.available()>0)
  {
    timer_Blu2_Now =  millis();
    Protocol = "BLU2";
    digitalWrite(led2,LOW);
    data_BluRx = Blu_gate2.readStringUntil('\n');
    // Wif_gate.println(data_BluRx);
    // Blu_gate1.println(data_BluRx);
    getDataRx(data_BluRx, PreBlu2);
    String DeleteBuf = Blu_gate2.readString();
    digitalWrite(led2,HIGH);
  }

  //Cổng Sub_Ghz
  CC1101_RxData();

  //  String test = "SubGHzNode,TEMP:23.32,HUMI:68.92,LIGHT:59.95,OFF";
  //  Protocol = "SUB";
  //  getDataRx(test, PreSub);
  //  delay(5000);

  //thay đổi kết nối
  getSttJson();
  //delay(2000);
}


/*-----------------------Chương trình con----------------------------------*/
void getDataRx(String str, PreData gate)
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
  x = str.substring(0,str.indexOf("\r")-4);
  if(str=="0"){
    Stt = "OFF";
  }
  else if(str=="1"){
    Stt = "ON";
  }else{
    Stt = str;
  }

 if(CheckData(gate)){
   DataJsonUART();
 }
  x = "";
  ClearVal();
}
void DataJsonUART(){
  //"SUB,DHTNode,32.60,72.00,0.00,OFF";
  String msgESP = Protocol + "," + Device + "," + Temp + "," + Humi + "," + Light + "," + Stt; 
  Wif_gate.println(msgESP);
  Wif_gate.flush();
  /*String msgESP =   "{\"P\":\"" + Protocol + "\"," +
                    "\"N\":\"" + Device + "\"," +
                    "\"S\":\"" + Stt + "\"," +
                    "\"T\":\"" + Temp + "\"," +
                    "\"H\":\"" + Humi + "\"," +
                    "\"L\":\"" + Light + "\"}";

  DynamicJsonDocument dataJson(1024);
  deserializeJson(dataJson, msgESP);
  serializeJson(dataJson,Wif_gate);
  serializeJson(dataJson,Blu_gate1);*/
}

int getSttConnect(unsigned long timerNow){
  if(millis()-timerNow > 20000){
    return 0;
  }
  else{return 1;}
}

void getSttJson(){
  Blu1_c = getSttConnect(timer_Blu1_Now);
  Blu2_c= getSttConnect(timer_Blu2_Now);
  Sub_c= getSttConnect(timer_Sub_Now);   
  //{"Json":"connect"}
  if((Blu1_c!=PreBlu1_c) || (Blu2_c!=PreBlu2_c) || (Sub_c!= PreSub_c)){
    String str = "connect";
    String msgConnect =   "{\"Json\":\"" + str + "\"," +
                          "\"BL1_c\":\"" + String(Blu1_c) + "\"," +
                          "\"BL2_c\":\"" + String(Blu2_c) + "\"," +
                          "\"SUB_c\":\"" + String(Sub_c) + "\"}";

    DynamicJsonDocument SttJson(1024);
    deserializeJson(SttJson, msgConnect);
    serializeJson(SttJson, Wif_gate);
    serializeJson(SttJson, Blu_gate1);
    PreBlu1_c = Blu1_c;
    PreBlu2_c = Blu2_c;
    PreSub_c  = Sub_c;
    
  } 
}

void ClearVal()
{
  Protocol = "";
  Device = "";
  Stt = "";
  Temp = "";
  Humi = "";
  Light = "";
  data_BluRx="";
  data_SubRx="";
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

void CC1101_TxData(String str_cmd){
  int n= str_cmd.length()+1;
  char arr_cmd[n];
  str_cmd.toCharArray(arr_cmd,n);
  if (radio.sendPacket(arr_cmd)) {
    Blu_gate1.println("Send control successed! :" + str_cmd);
  } 
  else {
    Blu_gate1.println("Ping failed due to high RSSI and/or incoming packet");
  }
}
void CC1101_RxData(){
    byte pkt_size = radio.getPacket(CC1101_rx);
    if (pkt_size>0 && radio.crcok()) { // We have a valid packet with some data
        timer_Sub_Now =  millis();
        // Wif_gate.print("Got packet \"");
        // Wif_gate.write(CC1101_rx, pkt_size);
        // Blu_gate1.print("\" len=");
        // Blu_gate1.print(pkt_size);
        // Blu_gate1.print(" Signal="); // for field tests to check the signal strength
        // Blu_gate1.print(radio.getRSSIdbm());
        // Blu_gate1.print(" LQI="); // for field tests to check the signal quality
        // Blu_gate1.println(radio.getLQI()); 
        
        String str;
        for(int i=0; i<pkt_size;i++)
        {
          str = String(char(CC1101_rx[i]));
          data_SubRx+=str;
        }
        Protocol = "SUB";
        getDataRx(data_SubRx, PreSub);
        ClearVal();  
      }    
}

void getCtr_Stm(String Ctr)
{
  //Ctr:{"Pro":"BLU1","Stt":"off"}
  String DVCtr,DVStt="";
  DynamicJsonDocument cmdJson(1024);
  DeserializationError error = deserializeJson(cmdJson, Ctr);  
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
    if (DVCtr == "BLU1"){
      Blu_gate1.println(DVStt);
    }
    else if(DVCtr == "BLU2"){
      Blu_gate2.println(DVStt);
    }
    else if(DVCtr =="SUB"){
      CC1101_TxData(DVStt);
    }
  }   
}

int CheckData(PreData gate){
  if((gate.PreTemp!=Temp)||(gate.PreHumi!=Humi)||(gate.PreLight!=Light)||(gate.PreStt!=Stt)){
    gate.PreTemp = Temp;
    gate.PreHumi  = Humi;
    gate.PreLight = Light;
    gate.PreStt = Stt;
    return 1;
  }
  else{
    return 0;
  }
}

