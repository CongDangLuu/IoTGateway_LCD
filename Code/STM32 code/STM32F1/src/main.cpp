#include <Arduino.h>
#include <HardwareSerial.h>
#include <string.h>
#include <ArduinoJson.h>
#include <SPI.h>
#include <CC1101_RF.h>
#include <interrupt.h>
#define led1 PB13
#define led2 PB12
#define ledonboard PC13

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

  PreData PreBlu[2], PreSub[5];

  typedef struct{
    uint32_t id[5];
    int cnt[5]={0,0,0,0,0};
    int Pre_cnt[5]={0,0,0,0,0};
  }Infor;

Infor Wifi_c, Sub_c, Blu_c;
void set_infor();

String data_BluRx, data_SubRx = "";// biến nhận chuỗi dữ liệu từ các node

String data_control = "";// biến nhận chuỗi Json điều khiển từ esp.

// int Blu1_c, Blu2_c, Sub_c = 0; 
// int PreBlu1_c, PreBlu2_c, PreSub_c = 0; 

unsigned long timer;
unsigned long timer_Blu1_Now, timer_Blu2_Now, CC1101_WaitTimer, CC1101_CheckACKTimer, SubTimer = 0;

void getDataRx(String str, PreData gate);                                                                 // xử lý dữ liệu nhận được từ bluetooth và subghz


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
  pinMode(ledonboard, OUTPUT);
  digitalWrite(ledonboard, HIGH);
  digitalWrite(led1, HIGH);
  digitalWrite(led2, HIGH);
  set_infor();
  SPI.begin(); 
  radio.begin(433.2e6); 
  radio.setRXstate(); 
  Wif_gate.println("Start............................");
  }

void loop() {
  
  //nhận lệnh điều khiển
  if(Wif_gate.available()>0)
  {
    data_control = Wif_gate.readStringUntil('\n'); //BLU1,1}
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
    Blu_gate1.println("nhan xong");
    getDataRx(data_BluRx,PreBlu[0]);
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
  
    getDataRx(data_BluRx, PreBlu[1]);
    String DeleteBuf = Blu_gate2.readString();
    digitalWrite(led2,HIGH);
  }

  //Cổng Sub_Ghz
  if(millis()-SubTimer>5000){
    CC1101_RxData();
    SubTimer = millis();
  }
  

  //thay đổi kết nối
  getSttJson();
  
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
    String msgESP = Protocol + "," + Device + "," + Temp + "," + Humi + "," + Light + "," + Stt; 
    Wif_gate.println(msgESP);
 }
  x = "";
  ClearVal();
}


int getSttConnect(unsigned long timerNow){
  if(millis()-timerNow > 20000){
    return 0;
  }
  else{return 1;}
}

void getSttJson(){
  Blu_c.cnt[0] = getSttConnect(timer_Blu1_Now);
  Blu_c.cnt[1]= getSttConnect(timer_Blu2_Now);
   //"connect,1,1,0,0,0,0,0"
  for (int i = 0; i < 5; i++){
    if((Blu_c.cnt[i]!=Blu_c.Pre_cnt[i]) || (Sub_c.cnt[i]!=Sub_c.Pre_cnt[i])){     
      String CntESP = "connect," + String(Blu_c.cnt[0])
                                + "," + String(Blu_c.cnt[1]) 
                                + "," + String(Sub_c.cnt[0])
                                + "," + String(Sub_c.cnt[1])
                                + "," + String(Sub_c.cnt[2])
                                + "," + String(Sub_c.cnt[3])
                                + "," + String(Sub_c.cnt[4]);
      Wif_gate.println(CntESP);
      for (int j = 0; j < 5; j++)
      {
        Blu_c.Pre_cnt[j] = Blu_c.cnt[j];
        Sub_c.Pre_cnt[j] = Sub_c.cnt[j];
      }
      
      break;
    }
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
  bool success = radio.sendPacket(arr_cmd);
  while(!(success)){
    success = radio.sendPacket(arr_cmd);
  }
  delay(1);
}
void CC1101_RxData(){   
  for(int i =0; i<5; i++){
    String respond = "request,"+String(Sub_c.id[i])+",";
    CC1101_TxData(respond);
    CC1101_WaitTimer = millis();
    CC1101_CheckACKTimer =  millis();
    byte pkt_size = radio.getPacket(CC1101_rx);
    while(!(pkt_size>0 && radio.crcok())){
      pkt_size = radio.getPacket(CC1101_rx);
      //gửi lại 3 lần
      if(millis()-CC1101_CheckACKTimer>300){
        respond = "repond,"+String(Sub_c.id[i])+",null";
        CC1101_TxData(respond);
        CC1101_CheckACKTimer = millis();
      }

      if(millis() - CC1101_WaitTimer> 2000){
        Sub_c.cnt[i]=0;       
        break;
      }
    }     

    if(pkt_size>0 && radio.crcok()){
      
      data_SubRx ="";
      for(int j=0; j<pkt_size;j++){
        data_SubRx += String(char(CC1101_rx[j]));
      }
      
      String ID_node = data_SubRx.substring(0,data_SubRx.indexOf(','));
      data_SubRx.remove(0,data_SubRx.indexOf(',')+1);
      if (ID_node == String(Sub_c.id[i]))
      { 
        Sub_c.cnt[i]=1;
        Protocol = "SUB" + String(i+1); 
        getDataRx(data_SubRx, PreSub[i]);
        ClearVal();
      } 
    }    
  }
}

void getCtr_Stm(String Ctr)
{
  String DVCtr,DVStt="";
  DVCtr = Ctr.substring(0,Ctr.indexOf(','));
  Ctr.remove(0,Ctr.indexOf(',')+1);
  DVStt = Ctr.substring(0,Ctr.indexOf("\r")-4);

  if (DVCtr == "BLU1"){
    Blu_gate1.println(DVStt);
  }
  else if(DVCtr == "BLU2"){
    Blu_gate2.println(DVStt);
  }
  else{
    for (int i = 0; i < 5; i++)
    {
      String gate = "SUB"+ String(i+1);
      if(DVCtr == gate){
        String control = "control,"+ String(Sub_c.id[i]) + "," + DVStt;
        Wif_gate.println(control);
        CC1101_TxData(control);
      }
    }
  }
    int a = DVStt.toInt();
    digitalWrite(ledonboard, !a);
    
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

  

