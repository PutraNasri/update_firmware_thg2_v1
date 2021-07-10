#include <Thread.h>
#include <ESP8266WiFi.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <ESP8266HTTPClient.h>
#include <ESP8266httpUpdate.h>
#include <WiFiClient.h>
#include <Arduino_JSON.h>
#include <WiFiManager.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#define ONE_WIRE_BUS 0
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensorSuhu(&oneWire);
float suhuSekarang;
//#include <SPI.h>
#include <SD.h>
///////////////////////////////////
#include <Wire.h>
#include "RTClib.h"
#include <Arduino.h>
#include <U8g2lib.h>
#ifdef U8X8_HAVE_HW_SPI
#include <SPI.h>
#endif
#ifdef U8X8_HAVE_HW_I2C
#include <Wire.h>
#endif
U8G2_SSD1327_EA_W128128_F_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE);  /* Uno: A4=SDA, A5=SCL, add "u8g2.setBusClock(400000);" into setup() for speedup if possible */
//////////////////////////////////
extern "C"
{
  #include <lwip/icmp.h> 
}

File myFile;
#if defined(ARDUINO_ARCH_SAMD)
   #define Serial SerialUSB
#endif
RTC_DS1307 rtc;
char daysOfTheWeek[7][12] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};


void setup() {
  #ifndef ESP8266
    while (!Serial); // for Leonardo/Micro/Zero
  #endif
  Serial.begin(115200);
  sensorSuhu.begin();
 
  
  u8g2.begin();
  u8g2.clearBuffer();
  u8g2.clearBuffer();
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_logisoso20_tr); // choose a suitable font 42 pixel
  u8g2.setCursor(2,52);
  u8g2.print("loading....");
  u8g2.sendBuffer(); 
  if (! rtc.begin()) {
//    Serial.println("Couldn't find RTC");    
    while (1);
  }

  if (! rtc.isrunning()) {
   
    // following line sets the RTC to the date & time this sketch was compiled
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
    // This line sets the RTC with an explicit date & time, for example to set
    // January 21, 2014 at 3am you would call:
    // rtc.adjust(DateTime(2014, 1, 21, 3, 0, 0));
    Serial.println("RTC ERROR");
    delay(1000);
//    ESP.restart();
  }

  Serial.println("RTC OK");
 
 
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }
  
  if (!SD.begin(15)) {
    Serial.println("SD ERROR");
  }else{
    Serial.println("SD OK");
  }
  
   delay(1000);

  if(SD.exists("id_device.txt")){
    
  }else{
    Serial.println("baca ok");
    service_lcd();
    delay(5000);
    ESP.restart();   
  }

  

  // put your setup code here, to run once:
}

void loop() {
  // put your main code here, to run repeatedly:

}

void service_lcd(){
  String tempp = String(getTemp()+00);
  Serial.println(tempp);
  char * temp = strdup(tempp.c_str());
  
  
  char * depan_temp = strtok(temp,".");
  char * belakang_temp = strtok(NULL,".");
  

  u8g2.clearBuffer();          // clear the internal memory
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_logisoso46_tn); // choose a suitable font 42 pixel
  u8g2.setCursor(2,52);
  
  u8g2.print(String(depan_temp+00));
  
  u8g2.setFont(u8g2_font_logisoso26_tr); // choose a suitable font 42 pixel
  u8g2.setCursor(85,25);
  u8g2.print(","+String(belakang_temp));
  
  u8g2.setCursor(100,60);
  u8g2.print("C");

  u8g2.setFont(u8g2_font_logisoso20_tr);
  u8g2.setCursor(20,97);
  u8g2.print("otori.id");

  u8g2.sendBuffer(); 

  yield();
}

float getTemp(){
  sensorSuhu.requestTemperatures();
  float suhu = sensorSuhu.getTempCByIndex(0);
  return suhu;

}
