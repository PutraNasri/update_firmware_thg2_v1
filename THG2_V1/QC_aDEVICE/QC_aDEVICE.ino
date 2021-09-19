#include <Thread.h>
#include <ESP8266WiFi.h>
#include "DHTesp.h"
//#include <SPI.h>
#include <SD.h>
//DHTesp dht;
///////////////////////////////////
#include "DHT.h"
#define DHTPIN 0
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);
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



/////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////bagian yang harus di sesuaikan////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////
String id_device      = "NVR73180";  /////////////////////////////////////////////////////////////////
const char* ssid      = "NVR73180";  /////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////

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
  dht.begin();
  u8g2.begin();
  u8g2.clearBuffer();
  u8g2.clearBuffer();
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_logisoso20_tr); // choose a suitable font 42 pixel
  u8g2.setCursor(2,52);
  u8g2.print("loading....");
  u8g2.sendBuffer(); 
  delay(2000);
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
//    Serial.println("RTC ERROR");
    u8g2.clearBuffer();
    u8g2.setFont(u8g2_font_logisoso20_tr); // choose a suitable font 42 pixel
    u8g2.setCursor(2,52);
    u8g2.print("RTC ERR");
    u8g2.sendBuffer(); 
    delay(2000);
    ESP.restart();
  }

//  Serial.println("RTC OK");
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_logisoso20_tr); // choose a suitable font 42 pixel
  u8g2.setCursor(2,52);
  u8g2.print("RTC OK");
  u8g2.sendBuffer(); 
  delay(2000);
 
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }
  
  if (!SD.begin(15)) {
//    Serial.println("SD ERROR");
    u8g2.clearBuffer();
    u8g2.setFont(u8g2_font_logisoso20_tr); // choose a suitable font 42 pixel
    u8g2.setCursor(2,52);
    u8g2.print("SD ERR");
    u8g2.sendBuffer(); 
    delay(2000);
    ESP.restart();
  }else{
//    Serial.println("SD OK");
    u8g2.clearBuffer();
    u8g2.setFont(u8g2_font_logisoso20_tr); // choose a suitable font 42 pixel
    u8g2.setCursor(2,52);
    u8g2.print("SD OK");
    u8g2.sendBuffer(); 
    delay(2000);
  }
  
   delay(2000);

  if(SD.exists("id_device.txt")){
//    Serial.println("baca ok");
    u8g2.clearBuffer();
    u8g2.setFont(u8g2_font_logisoso20_tr); // choose a suitable font 42 pixel
    u8g2.setCursor(2,52);
    u8g2.print("READ OK");
    u8g2.sendBuffer(); 
    delay(5000);
//    ESP.restart(); 
    service_sensor();  
    
  }else{
//    Serial.println("baca ok");
    u8g2.clearBuffer();
    u8g2.setFont(u8g2_font_logisoso20_tr); // choose a suitable font 42 pixel
    u8g2.setCursor(2,52);
    u8g2.print("READ OK");
    u8g2.sendBuffer(); 
    delay(5000);
//    ESP.restart();  
    service_sensor(); 
  }

  

  // put your setup code here, to run once:
}

void loop() {
  // put your main code here, to run repeatedly:

}

void service_sensor(){
  String tempp = String(dht.readTemperature());
  char * temp = strdup(tempp.c_str());
  String humm =String(dht.readHumidity());
  char * hum = strdup(humm.c_str());
//  Serial.println("temp"+tempp);
  if (tempp == "nan"){
    u8g2.clearBuffer(); 
    u8g2.clearBuffer();
    u8g2.setFont(u8g2_font_logisoso20_tr); // choose a suitable font 42 pixel
    u8g2.setCursor(0,52);
    u8g2.print("SENSOR ERR");
    u8g2.sendBuffer(); 
    delay(2000);
//    write_id();
    ESP.restart();
    
  }else{
    u8g2.clearBuffer();          
  //(yy,xx)
    u8g2.setFont(u8g2_font_logisoso42_tr);
    u8g2.setCursor(2,43);
    char * depan_temp = strtok(temp,".");
    char * belakang_temp = strtok(NULL,".");
    char * depan_hum = strtok(hum,".");
    char * belakang_hum = strtok(NULL,".");
    u8g2.print(String(depan_temp)+",");     
    u8g2.setFont(u8g2_font_logisoso20_tr);
    u8g2.setCursor(98,28);
    u8g2.print(".");
    u8g2.setCursor(105,43);
    u8g2.print("C");
    u8g2.setCursor(92,20);
    u8g2.print(belakang_temp);    
    u8g2.setFont(u8g2_font_logisoso42_tr);
    u8g2.setCursor(2,95);
    u8g2.print(String(depan_hum)+",");  
    u8g2.setFont(u8g2_font_logisoso20_tr);
    u8g2.setCursor(105,96);
    u8g2.print("%");
    u8g2.setCursor(92,74);
    u8g2.print(belakang_hum);
    u8g2.sendBuffer(); 
    delay(2000);
    write_id();
//    ESP.restart();   
  } 
  yield(); 
}

void write_id(){  
  u8g2.clearBuffer(); 
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_logisoso20_tr); // choose a suitable font 42 pixel
  u8g2.setCursor(0,52);
  u8g2.print("WRT ID");
  u8g2.sendBuffer(); 
  delay(2000);
  SD.remove("id_device.txt"); 
  delay(2000);
  Serial.println("menulis id_device to sdcard");
  myFile = SD.open("id_device.txt", FILE_WRITE);
  if(myFile){  
   Serial.print("Writing to id_device.txt...");
   myFile.println(id_device);
   myFile.close();
   Serial.println("done.");
   u8g2.clearBuffer(); 
   u8g2.clearBuffer();
   u8g2.setFont(u8g2_font_logisoso20_tr); // choose a suitable font 42 pixel
   u8g2.setCursor(0,52);
   u8g2.print("ID OK");
   u8g2.sendBuffer(); 
   delay(2000);
   myFile = SD.open("id_device.txt");
   String id_device_sdcard;
   int count=1;
   if(myFile){
    while(myFile.available()){
      id_device_sdcard = String(myFile.readStringUntil('\n'));
      id_device_sdcard.trim();       
      
//      ssid      = id_device.c_str();      
      delay(3000);  
    }
      myFile.close();
      
      
      u8g2.setFont(u8g2_font_logisoso16_tr); // choose a suitable font 42 pixel
      u8g2.setCursor(2,90);
      u8g2.print(id_device.c_str());
      u8g2.sendBuffer(); 

    }else{
      u8g2.clearBuffer();
      u8g2.clearBuffer();
      u8g2.setFont(u8g2_font_logisoso20_tr); // choose a suitable font 42 pixel
      u8g2.setCursor(2,52);
      u8g2.print("RD ID ERR");
      u8g2.sendBuffer(); 
      delay(2000);  
      ESP.restart();   
    }
  }else{
    Serial.println("error opening test.txt");
    u8g2.clearBuffer(); 
    u8g2.clearBuffer();
    u8g2.setFont(u8g2_font_logisoso20_tr); // choose a suitable font 42 pixel
    u8g2.setCursor(0,52);
    u8g2.print("WRT ERR");
    u8g2.sendBuffer(); 
    delay(2000);
    ESP.restart();   
  }
   
}
