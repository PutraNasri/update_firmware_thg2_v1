#include <Arduino.h>
#include <WiFiManager.h>
#include <NTPClient.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <Arduino_JSON.h>
#include <WiFiUdp.h>
#include <SD.h>
#include <Wire.h>
#include "RTClib.h"
#include <SPI.h>

const char* remote_host = "https://otori-279015.df.r.appspot.com";
String get_pemilik = "https://otori-279015.df.r.appspot.com/log/pemilik";
String get_adjustment = "https://otori-279015.df.r.appspot.com/log/adjustment";
String get_delay = "https://otori-279015.df.r.appspot.com/log/delay";
String get_versionfirmware = "https://otori-279015.df.r.appspot.com/log/versionfirmware";
String post_data = "https://otori-279015.df.r.appspot.com/log";
String Fingerprint = "null";
#define URL_update "https://raw.githubusercontent.com/PutraNasri/update_firmware_thg2_v1/main/THG2_V1/ini.bin"

String id_device      = "TESTINGDATA";
const char* ssid      = "TESTINGDATA";
const char* password = "12345678";
String sts_server = "0";
String delay_server = "1";
String pemilik = "";
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP);
File myFile;
String formattedDate;
String dayStamp;
String timeStamp;

RTC_DS1307 rtc;
char daysOfTheWeek[7][12] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};

// lakukan pengecekan isi config.txt
  //jika tidak ada maka lakukan download data config.txt
  //jika ada maka pass
//pengecekan data suhu sesuai delay yang telah di tentukan
//lakukan pengiriman ke cloud
  //check sd dulu
    //jika sd ada data maka cek koneksi jika ok kirim data  push
  //jika =!200  maka simpan ke sd card



void Task1(void *parameter){

  for( ;; ){
    
    Serial.println("Connecting to ");
    WiFiManager wifiManager;
    wifiManager.setTimeout(60);
    wifiManager.setBreakAfterConfig(true);
    if (!wifiManager.autoConnect(ssid, password)) {
      Serial.println("failed to connect, we should reset as see if it connects");
      continue;
    } 
    
    Serial.println("");
    Serial.println("WiFi connected.");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());
    vTaskDelay(40000 / portTICK_PERIOD_MS); 
    
  }
}

void Task2(void *parameter){
  for( ;; ){
    delay(10000);
    
    if(WiFi.status() == WL_CONNECTED){
      String pemilik_ping = httpPOSTRequest_pemilik();
      JSONVar var_pemilik_ping = JSON.parse(pemilik_ping);
      pemilik_ping = var_pemilik_ping["pemilik"];

      if(pemilik_ping ==""){     
        Serial.println("pemilik kosong");
//      ESP.restart(); 
      }else{  
        String delay_server_control = httpPOSTRequest_delay();
        JSONVar var_delay_control = JSON.parse(delay_server_control);
        delay_server_control = var_delay_control["delay"];
//        Serial.println("delay control = "+delay_server_control);
        delay(1000);     
        String sts_adjustment_rh_temp_control = httpPOSTRequest_adjustment();
        JSONVar var_adjustment_rh_temp_control =JSON.parse(sts_adjustment_rh_temp_control);
        sts_adjustment_rh_temp_control = var_adjustment_rh_temp_control["adjustment_rh_temp"];
//        Serial.println("adjustment control = "+sts_adjustment_rh_temp_control);
        delay(1000);    
        String firmware_version_control = get_version_firmware();
        JSONVar var_firmware_version = JSON.parse(firmware_version_control);
        firmware_version_control = var_firmware_version["versi_firmware"];
//        Serial.println("versi firmware control = "+firmware_version_control);
        delay(1000);
              
      }
    
    }else{
      Serial.println("error wifi");
    }
    
  vTaskDelay(50000 / portTICK_PERIOD_MS);
  }
}

void setup() {
  Serial.begin(115200);

  //setting rtc
  //setting sd
  

  Serial.println("Connecting to ");
  WiFiManager wifiManager;
  wifiManager.setTimeout(60);
  wifiManager.setBreakAfterConfig(true);
  if (!wifiManager.autoConnect(ssid, password)) {
    Serial.println("failed to connect, we should reset as see if it connects");
//    continue;
  }
  
  Serial.println("");
  Serial.println("WiFi connected.");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());


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
    
  }
/////////////////////////////////////////////////////////////////////////////
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }
  
  if (!SD.begin(15)) {
    while (1);
  }
  Serial.println("initialization SD done.");
  delay(1000);

  if(SD.exists("id_device.txt")){
    myFile = SD.open("id_device.txt");
    String id_device_sdcard;
    int count=1;
    if(myFile){
      while(myFile.available()){
        id_device_sdcard = String(myFile.readStringUntil('\n'));
        id_device_sdcard.trim();       
        id_device = id_device_sdcard;
        ssid      = id_device.c_str();   
        
        delay(3000);  
      }
      myFile.close();
      Serial.println(ssid);
      //tambahkan print id di lcd 
      
    }else{
      delay(1000);     
    }
  }else{ 
  }

  ////////////////////////////////////////////////////////////////////////  
  timeClient.begin();
  timeClient.setTimeOffset(25200);
////////////////////////////////////////////////////////////////
  if(SD.exists("config.txt")){   
    myFile = SD.open("config.txt");
    String data_config;
    int count=1;
    String id_pemilik_config,delay_config,version_config;
    if(myFile){
      
      int n = 0;
      while(myFile.available()){
        data_config = String(myFile.readStringUntil('\n'));
        data_config.trim();
        if(n == 0){
          id_pemilik_config = data_config;
          }
        if(n == 1){
          delay_config = data_config;
        }
        if(n==2){
          version_config = data_config;
        }
       n += 1;       
        delay(3000);  
      }
      myFile.close();     
      
      delay_server = delay_config;
      pemilik = id_pemilik_config; 
      Serial.println("setting pemilik ok");   
      Serial.println("pemilik = "+pemilik);
    }else{      
      Serial.println("error sd");
    }
  }else{
    if(WiFi.status() == WL_CONNECTED){      
      String pemilik_ping = httpPOSTRequest_pemilik();
      JSONVar var_pemilik_ping = JSON.parse(pemilik_ping);
      pemilik_ping = var_pemilik_ping["pemilik"];
     
      if(pemilik_ping ==""){        
        Serial.println("error ping");
        delay (3000) ;
        ESP.restart();            
      }else{ 
        sinkronisasi_waktu();     

        String pemilik = httpPOSTRequest_pemilik();
        JSONVar var_pemilik = JSON.parse(pemilik);
        pemilik = var_pemilik["pemilik"];       
       
        delay(1000);
        String delay_server_control = httpPOSTRequest_delay();
        JSONVar var_delay_control = JSON.parse(delay_server_control);
        delay_server_control = var_delay_control["delay"];
                   
        delay(1000);
        String firmware_version_control = get_version_firmware();
        JSONVar var_firmware_version = JSON.parse(firmware_version_control);
        firmware_version_control = var_firmware_version["versi_firmware"];
             
        delay(1000);

        if(pemilik == ""){
          // ESP.restart(); 
        }
        if(delay_server_control == ""){
          // ESP.restart(); 
        }
        if(firmware_version_control == ""){
          // ESP.restart();  
        }       
        myFile = SD.open("config.txt", FILE_WRITE);
        if(myFile){
          myFile.println(pemilik);
          myFile.println(delay_server_control);
          myFile.println(firmware_version_control);
          myFile.close();
          // ESP.restart(); 
        }else{          
          Serial.println("error config");
          delay (3000) ;
          // ESP.restart(); 
        } 
      }
    }else{    
      Serial.println("need wifi");
      
      delay (3000) ; 
      // ESP.restart(); 
    }   
  }
  

  int delay_server_service = delay_server.toInt();

  
//  xTaskCreate(
//          Task1,
//          "Task 1",
//          5000,
//          NULL,
//          1,
//          NULL);

  xTaskCreate(
        Task2,
        "Task 2",
        5000,
        NULL,
        1,
        NULL);
          
//  xTaskCreatePinnedToCore(
//            Task1,
//            "Task 1",
//            5000,
//            NULL,
//            1,
//            NULL,
//            CONFIG_ARDUINO_RUNNING_CORE);

//   xTaskCreatePinnedToCore(
//            Task2,
//            "Task 2",
//            5000,
//            NULL,
//            1,
//            NULL,
//            CONFIG_ARDUINO_RUNNING_CORE);

}

String httpPOSTRequest_pemilik(){
  HTTPClient http;
  http.begin(get_pemilik);
  http.addHeader("Content-Type", "application/json");
  int body = http.POST("{\n\t\"id\":\"" + id_device + "\"\n}");
  String payload = http.getString();
  Serial.print("HTTP Response code pemilik: ");
  Serial.println(body);
  Serial.print("HTTP Response data pemilik: ");
  Serial.println(payload);
  http.end();

  if (body==200){
    return payload;
  }
  else{
    return String(body);
  }

}

String httpPOSTRequest_delay(){
  HTTPClient http;
  http.begin(get_delay);
  http.addHeader("Content-Type", "application/json");
  int body = http.POST("{\n\t\"id\":\"" + id_device + "\"\n}");
  String payload = http.getString();
  Serial.print("HTTP Response code delay: ");
  Serial.println(body);
  Serial.print("HTTP Response data delay: ");
  Serial.println(payload);
  http.end();

  if (body==200){
    return payload;
  }
  else{
    return String(body);
  }
  
 
}

String httpPOSTRequest_adjustment(){
  HTTPClient http;
  http.begin(get_adjustment);
  http.addHeader("Content-Type", "application/json");
  int body = http.POST("{\n\t\"id\":\"" + id_device + "\"\n}");
  String payload = http.getString();
  Serial.print("HTTP Response code adjustment: ");
  Serial.println(body);
  Serial.print("HTTP Response data adjustment: ");
  Serial.println(payload);
  http.end();

  if (body==200){
    return payload;
  }
  else{
    return String(body);
  }
  
//  return payload;
 
}

String get_version_firmware(){
  HTTPClient http;
  http.begin(get_versionfirmware);
  http.addHeader("Content-Type", "application/json");
  int body = http.POST("{\n\t\"id\":\"" + id_device + "\"\n}");
  String payload = http.getString();
  Serial.print("HTTP Response code versiofirmware: ");
  Serial.println(body);
  Serial.print("HTTP Response data versiofirmware: ");
  Serial.println(payload);
  http.end();
  
  if (body==200){
    return payload;
  }
  else{
    return String(body);
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////
void sinkronisasi_waktu(){
  while(!timeClient.update()) {
      timeClient.forceUpdate();
  }
  formattedDate = timeClient.getFormattedDate();
  int splitT = formattedDate.indexOf("T");
  dayStamp = formattedDate.substring(0, splitT);
  timeStamp = formattedDate.substring(splitT+1, formattedDate.length()-1);       
  
  char *tahun; 
  tahun = strtok((char *) dayStamp.c_str(),"-");        
  char *bulan = strtok(NULL,"-");
  char *hari = strtok(NULL,"-");
  char *jam;
  jam = strtok((char *) timeStamp.c_str(),":");
  char *menit = strtok(NULL,":");
  char *detik = strtok(NULL,":");
  String tahun_str = String(tahun);
  String bulan_str = String(bulan);
  String hari_str = String(hari);
  String jam_str = String(jam);
  String menit_str = String(menit);
  String detik_str = String(detik);      
  rtc.adjust(DateTime(tahun_str.toInt(), bulan_str.toInt(), hari_str.toInt(), jam_str.toInt(), menit_str.toInt(), detik_str.toInt()));
  Serial.println("berhasil singkroninasi");
  
}

//fction read time

//fction read sensor 1

//fction singkronisasi data sd

//fction button reset

//fction led check

//fction sleep


void loop() {
  
}
