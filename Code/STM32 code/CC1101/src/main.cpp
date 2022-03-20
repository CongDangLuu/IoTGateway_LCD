
#include <Arduino.h>
#include <ELECHOUSE_CC1101.h>
#include <HardwareSerial.h>

HardwareSerial Blu_gate1(PA10, PA9);  // TX-PA10, RX-PA9
void setup() {
   Blu_gate1.begin(9600);
  ELECHOUSE_cc1101.Init(F_433);   
  Serial.println("Tx ..........");    
}
void loop() {
  
  float h ;    
  float t ; 
  h= 55.55;
  t= 30.2;
  String tx_message = String(t)+ "|" + String(h);
  int m_length = tx_message.length();
  byte txbyte[m_length];
  tx_message.getBytes(txbyte, m_length + 1);  //m_length + 1: cộng thêm 1 byte địa chỉ
  Blu_gate1.println((char *)txbyte);
  ELECHOUSE_cc1101.SendData(txbyte, m_length);
  delay(1000);                     
}
