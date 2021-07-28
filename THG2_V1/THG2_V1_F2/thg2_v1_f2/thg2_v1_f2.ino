///////////////////////
////author by Putra////
/////////THG2//////////
////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////
#include <Thread.h>
#include <ESP8266WiFi.h>
#include <ESP8266Ping.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <ESP8266HTTPClient.h>
#include <ESP8266httpUpdate.h>
#include <WiFiClient.h>
#include <Arduino_JSON.h>
#include <WiFiManager.h>
//#include "DHTesp.h"
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
//////////////////////////////////
const char* remote_host = "http://wb21.otori.id";
String get_pemilik = "http://wb21.otori.id/thg2/get_pemilik";
String get_adjustment_rh_temp = "http://wb21.otori.id/thg2/get_adjustment_rh_temp";
String get_delay = "http://wb21.otori.id/thg2/get_delay";
String get_versionfirmware = "http://wb21.otori.id/thg2/versi";
String post_data = "http://wb21.otori.id/thg2/nulis_data";
String Fingerprint = "null";
#define URL_update "https://raw.githubusercontent.com/PutraNasri/update_firmware_thg2_v1/main/THG2_V1/ini.bin"
HTTPClient http;
String id_device      = "XXXXXXXX";
const char* ssid      = "XXXXXXXX";
String version_firmware = "F2";
const char* password = "12345678";
String sts_server = "0";
String sts_adjustment_rh_temp = "0@0";
String delay_server = "1";
String pemilik = "";
float rh_call = 0;
float temp_call = 0;
int menit_to_detik = 60;
int detik_to_milidetik = 1000;
const int pin_sensor = 0;
const int btn_acpn = 16;
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP);
File myFile;
String formattedDate;
String dayStamp;
String timeStamp;
String indicator_api_200 = "0";
extern "C"
{
  #include <lwip/icmp.h> 
}
Thread Thread1 = Thread();
Thread Thread2 = Thread();
Thread Thread3 = Thread();
Thread Thread4 = Thread();
#if defined(ARDUINO_ARCH_SAMD)
   #define Serial SerialUSB
#endif
RTC_DS1307 rtc;
char daysOfTheWeek[7][12] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};


void setup () {
  #ifndef ESP8266
    while (!Serial); // for Leonardo/Micro/Zero
  #endif
  Serial.begin(115200);
  pinMode(DHTPIN, INPUT);
  dht.begin();
  
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
    
//    u8g2.clearBuffer();
//    u8g2.clearBuffer();
//    u8g2.setFont(u8g2_font_logisoso20_tr); // choose a suitable font 42 pixel
//    u8g2.setCursor(2,52);
//    u8g2.print("RTC error....");
//    u8g2.sendBuffer();
//    delay(1000);
//    ESP.restart();
  }
/////////////////////////////////////////////////////////////////////////////
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }
  
  if (!SD.begin(15)) {
//    u8g2.clearBuffer();
//    u8g2.clearBuffer();
//    u8g2.setFont(u8g2_font_logisoso20_tr); // choose a suitable font 42 pixel
//    u8g2.setCursor(2,52);
//    u8g2.print("SD error....");
//    u8g2.sendBuffer(); 
//    delay(1000);
//    ESP.restart();  
    while (1);
  }
  Serial.println("initialization SD done.");
  delay(1000);
////////////////////////////////////////////////////////////////////////////////////
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
      
      u8g2.setFont(u8g2_font_logisoso16_tr); // choose a suitable font 42 pixel
      u8g2.setCursor(2,90);
      u8g2.print(ssid);
      u8g2.sendBuffer(); 

    }else{
//      u8g2.clearBuffer();
//      u8g2.clearBuffer();
//      u8g2.setFont(u8g2_font_logisoso20_tr); // choose a suitable font 42 pixel
//      u8g2.setCursor(2,52);
//      u8g2.print("SD2 error....");
//      u8g2.sendBuffer(); 
      delay(1000);     
    }
  }else{
//    u8g2.clearBuffer();
//    u8g2.clearBuffer();
//    u8g2.setFont(u8g2_font_logisoso20_tr); // choose a suitable font 42 pixel
//    u8g2.setCursor(2,52);
//    u8g2.print("ID error....");
//    u8g2.sendBuffer(); 
//    delay(1000);
//    ESP.restart();   
  }
/////////////////////////////////////////////////////////////////////  

  WiFiManager wifiManager;
  wifiManager.setTimeout(60);
  wifiManager.setBreakAfterConfig(true);
  if (!wifiManager.autoConnect(ssid, password)) {
    
  }
////////////////////////////////////////////////////////////////////////  
  timeClient.begin();
  timeClient.setTimeOffset(25200);
////////////////////////////////////////////////////////////////
  if(SD.exists("config.txt")){   
    myFile = SD.open("config.txt");
    String data_config;
    int count=1;
    String id_pemilik_config,delay_config,adjustment_config,version_config;
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
        if(n == 2){
          adjustment_config = data_config;
        }
        if(n==3){
          version_config = data_config;
        }
       n += 1;       
        delay(3000);  
      }
      myFile.close();     
      
      sts_adjustment_rh_temp = adjustment_config;
      delay_server = delay_config;
      pemilik = id_pemilik_config; 
      Serial.println("setting pemilik ok");   
      Serial.println("pemilik = "+pemilik);
    }else{      
      u8g2.clearBuffer();
      u8g2.clearBuffer();
      u8g2.setFont(u8g2_font_logisoso20_tr); 
      u8g2.setCursor(2,52);
      u8g2.print("error config");
      u8g2.sendBuffer();
      delay(1000);
      ESP.restart();
    }   
  }else{ 
    if(WiFi.status() == WL_CONNECTED){      
      String pemilik_ping = httpPOSTRequest_pemilik();
      JSONVar var_pemilik_ping = JSON.parse(pemilik_ping);
      pemilik_ping = var_pemilik_ping["pemilik"];
     
      if(pemilik_ping ==""){        
        u8g2.clearBuffer();
        u8g2.setFont(u8g2_font_logisoso20_tr); // choose a suitable font 42 pixel
        u8g2.setCursor(2,52);
        u8g2.print("error ping...");
        u8g2.sendBuffer();
        delay (3000) ;
        ESP.restart();            
      }else{ 
        sinkronisasi_waktu();     
//        timeClient.begin();
//        timeClient.setTimeOffset(25200);       
//        while(!timeClient.update()) {
//            timeClient.forceUpdate();
//        }
//        formattedDate = timeClient.getFormattedDate();
//        int splitT = formattedDate.indexOf("T");
//        dayStamp = formattedDate.substring(0, splitT);
//        timeStamp = formattedDate.substring(splitT+1, formattedDate.length()-1);       
//        
//        char *tahun; 
//        tahun = strtok((char *) dayStamp.c_str(),"-");        
//        char *bulan = strtok(NULL,"-");
//        char *hari = strtok(NULL,"-");
//        char *jam;
//        jam = strtok((char *) timeStamp.c_str(),":");
//        char *menit = strtok(NULL,":");
//        char *detik = strtok(NULL,":");
//        String tahun_str = String(tahun);
//        String bulan_str = String(bulan);
//        String hari_str = String(hari);
//        String jam_str = String(jam);
//        String menit_str = String(menit);
//        String detik_str = String(detik);      
//        rtc.adjust(DateTime(tahun_str.toInt(), bulan_str.toInt(), hari_str.toInt(), jam_str.toInt(), menit_str.toInt(), detik_str.toInt()));

        String pemilik = httpPOSTRequest_pemilik();
        JSONVar var_pemilik = JSON.parse(pemilik);
        pemilik = var_pemilik["pemilik"];       
       
        delay(1000);
        String delay_server_control = httpPOSTRequest_delay();
        JSONVar var_delay_control = JSON.parse(delay_server_control);
        delay_server_control = var_delay_control["delay"];
               
        delay(1000);
        String sts_adjustment_rh_temp_control = httpPOSTRequest_adjustment_rh_temp();
        JSONVar var_adjustment_rh_temp_control =JSON.parse(sts_adjustment_rh_temp_control);
        sts_adjustment_rh_temp_control = var_adjustment_rh_temp_control["adjustment_rh_temp"];
             
        delay(1000);
        String firmware_version_control = get_version_firmware();
        JSONVar var_firmware_version = JSON.parse(firmware_version_control);
        firmware_version_control = var_firmware_version["versi_firmware"];
             
        delay(1000);

        if(pemilik == ""){
          ESP.restart(); 
        }
        if(delay_server_control == ""){
          ESP.restart(); 
        }
        if(sts_adjustment_rh_temp_control == ""){
          ESP.restart(); 
        }
        if(firmware_version_control == ""){
          ESP.restart();  
        }       
        myFile = SD.open("config.txt", FILE_WRITE);
        if(myFile){
          myFile.println(pemilik);
          myFile.println(delay_server_control);
          myFile.println(sts_adjustment_rh_temp_control);
          myFile.println(firmware_version_control);
          myFile.close();
          ESP.restart(); 
        }else{          
          u8g2.clearBuffer();
          u8g2.clearBuffer();
          u8g2.setFont(u8g2_font_logisoso20_tr); // choose a suitable font 42 pixel
          u8g2.setCursor(2,52);
          u8g2.print("error w config");
          u8g2.sendBuffer();
          delay (3000) ;
          ESP.restart(); 
        } 
      }
    }else{    
      u8g2.clearBuffer();
      u8g2.clearBuffer();
      u8g2.setFont(u8g2_font_logisoso20_tr); // choose a suitable font 42 pixel
      u8g2.setCursor(2,52);
      u8g2.print("need wifi..");
      u8g2.sendBuffer();
      delay (3000) ; 
      ESP.restart(); 
    }   
  }
  char *sts_adjustment_rh_tempp = strdup(sts_adjustment_rh_temp.c_str());
  char * rh = strtok(sts_adjustment_rh_tempp,"@");
  char * temp = strtok(NULL,"@");
  rh_call=atof(rh);
  temp_call=atof(temp); 

  int delay_server_service = delay_server.toInt();

  // panggil service() untuk merecord data pada saat awal start device
//  service();
  delay(2000);
  Thread1.onRun(service);
  Thread1.setInterval(delay_server_service*menit_to_detik*detik_to_milidetik);
  Thread2.onRun(acpn_mode);
  Thread2.setInterval(500);
  Thread3.onRun(service_control);
  Thread3.setInterval(180000);
  Thread4.onRun(service_lcd);
  Thread4.setInterval(10000);
  
  delay(2000);
  service();
}


void service(){
  if(WiFi.status() == WL_CONNECTED){  
//    delay(2000);   
    String pemilik_ping = httpPOSTRequest_pemilik();
    JSONVar var_pemilik_ping = JSON.parse(pemilik_ping);
    pemilik_ping = var_pemilik_ping["pemilik"];

     
    if (pemilik_ping ==""){
      Serial.println("ping pemilik gagal");
      tulis_sd_card();
      delay(1000);
//      ESP.restart(); 
    }else{    
      if(pemilik != ""){
        cek_data_sdcard_and_send_to_firebase();
        if(pemilik != "no_id_user" && pemilik != "lock"){
          sts_server = "1";
          cek_data_sdcard_and_send_to_firebase();
          float h = dht.readHumidity();
          float t = dht.readTemperature();

          if(String(h)=="nan"){
            u8g2.clearBuffer();
            u8g2.clearBuffer();
            u8g2.setFont(u8g2_font_logisoso20_tr); // choose a suitable font 42 pixel
            u8g2.setCursor(0,52);
            u8g2.print("snr1 err ....");
            u8g2.sendBuffer(); 
            Serial.println("snr1 err....");
            digitalWrite(DHTPIN,LOW);
            delay(1000);
            ESP.restart();
          }else{
            while(!timeClient.update()){
            timeClient.forceUpdate();
            }
            formattedDate = timeClient.getFormattedDate();
            int splitT = formattedDate.indexOf("T");
            dayStamp = formattedDate.substring(0, splitT);
            timeStamp = formattedDate.substring(splitT+1, formattedDate.length()-1);
            String data = String(t+temp_call)+"@"+String(h+rh_call)+"@"+String(dayStamp)+"@"+String(timeStamp);
            httpPOSTRequest_post_data(data); 
          }     
           
        }else if(pemilik=="no_id_user"){
          
        }else if(pemilik=="lock"){
          tulis_sd_card();         
        }
        yield();         
      }else{       
      }  
    } 
  }else{   
    tulis_sd_card();  
  }
  yield();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void acpn_mode(){
  if(digitalRead(btn_acpn)==HIGH){
      u8g2.clearBuffer();          
      u8g2.setFont(u8g2_font_logisoso42_tr); 
      u8g2.setCursor(2,42);
      u8g2.print("REST");
      u8g2.sendBuffer(); 
      WiFiManager wifiManager;
      wifiManager.resetSettings();
      wifiManager.setTimeout(60);
      wifiManager.setBreakAfterConfig(true);  
      wifiManager.autoConnect(ssid, password);      
  }
  yield();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void service_control(){
  if(WiFi.status() == WL_CONNECTED){
    delay(1000);
    
    String pemilik_ping = httpPOSTRequest_pemilik();
    JSONVar var_pemilik_ping = JSON.parse(pemilik_ping);
    pemilik_ping = var_pemilik_ping["pemilik"];
    
    if(pemilik_ping ==""){     
      sts_server = "0"; 
//      ESP.restart(); 
    }else{      
      String delay_server_control = httpPOSTRequest_delay();
      JSONVar var_delay_control = JSON.parse(delay_server_control);
      delay_server_control = var_delay_control["delay"];
      Serial.println("delay control = "+delay_server);
      delay(1000);     
      String sts_adjustment_rh_temp_control = httpPOSTRequest_adjustment_rh_temp();
      JSONVar var_adjustment_rh_temp_control =JSON.parse(sts_adjustment_rh_temp_control);
      sts_adjustment_rh_temp_control = var_adjustment_rh_temp_control["adjustment_rh_temp"];
//      Serial.println("adjustment control = "+sts_adjustment_rh_temp_control);
      delay(1000);    
      String firmware_version_control = get_version_firmware();
      JSONVar var_firmware_version = JSON.parse(firmware_version_control);
      firmware_version_control = var_firmware_version["versi_firmware"];
//      Serial.println("versi firmware control = "+firmware_version_control);
      delay(1000);    
      String fingerprint_control = get_version_firmware();
      JSONVar var_fingerprint = JSON.parse(fingerprint_control);
      fingerprint_control = var_fingerprint["fingerprint"];
//      Serial.println("versi fingerprint control = "+fingerprint_control);
      delay(1000);
      Fingerprint=fingerprint_control;
      if(delay_server_control !="" && sts_adjustment_rh_temp_control !="" && firmware_version_control !="" && fingerprint_control !="" && pemilik_ping !=""){
          if (delay_server != delay_server_control){          
            SD.remove("config.txt");  
            delay(1000);  
            ESP.restart(); 
          }else if(sts_adjustment_rh_temp != sts_adjustment_rh_temp_control){           
            SD.remove("config.txt");  
            delay(1000);
            ESP.restart(); 
          }else if(firmware_version_control != version_firmware){
            SD.remove("config.txt");  
            delay(1000);
            update_firmware();
          }else if(pemilik_ping != pemilik){
            SD.remove("config.txt");  
            delay(1000);
            ESP.restart();             
          }else{        
          }
          sinkronisasi_waktu();
      }else{ 
        Serial.println("ada data yg kosong");       
      }    
    }  
    yield();

  }else if (WiFi.status() != WL_CONNECTED){   
  }
  yield();
}
///////////////////////////////////////////////////////////////////////////////////////////////////
void service_lcd(){
  delay(2000);
  String tempp = String(dht.readTemperature()+temp_call);
  char * temp = strdup(tempp.c_str());
  String humm =String(dht.readHumidity()+rh_call);
  char * hum = strdup(humm.c_str());
  if (tempp == "nan"){
    u8g2.clearBuffer();
    u8g2.clearBuffer();
    u8g2.setFont(u8g2_font_logisoso20_tr); // choose a suitable font 42 pixel
    u8g2.setCursor(0,52);
    u8g2.print("snr2 err....");
    u8g2.sendBuffer(); 
    delay(1000);
    Serial.println("snr2 err....");
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
  } 
  yield(); 
}
////////////////////////////////////////////////////////////////////////////////////////////////
String httpPOSTRequest_pemilik(){
  delay(2000);
  HTTPClient http;
  http.begin(get_pemilik);
  http.addHeader("Content-Type", "application/json");
  int body = http.POST("{\n\t\"id\":\"" + id_device + "\"\n}");
  String payload = http.getString();
  
//  Serial.print("HTTP Response code pemilik: ");
//  Serial.println(body);
  
//  Serial.print("HTTP Response data pemilik: ");
//  Serial.println(payload);
  if (body==200){
    http.end();
    return payload;
  }
  else {
    http.end(); 
  }
  http.end();
}
////////////////////////////////////////////////////////////////////////////////////////////////
String httpPOSTRequest_delay(){
  HTTPClient http;
  http.begin(get_delay);
  http.addHeader("Content-Type", "application/json");
  int body = http.POST("{\n\t\"id\":\"" + id_device + "\"\n}");
  String payload = http.getString();
  
//  Serial.print("HTTP Response code delay: ");
//  Serial.println(body);
  
//  Serial.print("HTTP Response data delay: ");
//  Serial.println(payload);
  if (body==200){
    http.end();
    return payload;  
  }
  else {
    http.end();   
  }
  http.end();
}
////////////////////////////////////////////////////////////////////////////////////////////////
String httpPOSTRequest_adjustment_rh_temp(){
  HTTPClient http;
  http.begin(get_adjustment_rh_temp);
  http.addHeader("Content-Type", "application/json");
  int body = http.POST("{\n\t\"id\":\"" + id_device + "\"\n}");
  String payload = http.getString();
  
//  Serial.print("HTTP Response code adjustment: ");
//  Serial.println(body);
  
//  Serial.print("HTTP Response data adjustment: ");
//  Serial.println(payload);  
  if (body==200){
    http.end(); 
    return payload;
  }
  else {
    String error = "error request";
    http.end();     
  }
  http.end(); 
}
////////////////////////////////////////////////////////////////////////////////////////////////
String httpPOSTRequest_post_data(String data){
  HTTPClient http;
  http.begin(post_data);
  http.addHeader("Content-Type", "application/json");
  String params = "{\"id\":\""+id_device+"\",\"data\":\""+String(data)+"\"}";
  Serial.println(params);
  int body = http.POST(params);
  String payload = http.getString();
  
//  Serial.print("HTTP Response code post data: ");
//  Serial.println(body);
  
//  Serial.print("HTTP Response data post data: ");
//  Serial.println(payload); 
  if (body==200){
    Serial.println("ini data post api = "+data);
    http.end();  
    return payload; 
  }
  else {
    String error = "error request";
    http.end();
    Serial.println("error hit");
    tulis_sd_card();   
  }
  http.end();
}
////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////
String httpPOSTRequest_post_data_sd(String data){
  HTTPClient http;
  http.begin(post_data);
  http.addHeader("Content-Type", "application/json");
  String params = "{\"id\":\""+id_device+"\",\"data\":\""+String(data)+"\"}";
  Serial.println(params);
  int body = http.POST(params);
  String payload = http.getString();
  
//  Serial.print("HTTP Response code post data: ");
//  Serial.println(body);
  
//  Serial.print("HTTP Response data post data: ");
//  Serial.println(payload); 
  if (body==200){
    Serial.println("ini data post api = "+data);
    http.end();  
    indicator_api_200 = "200";
    return payload; 
  }
  else {
    String error = "error request";
    http.end();
    Serial.println("error hit");
    tulis_sd_card();   
    indicator_api_200 = "0";
  }
  http.end();
}
////////////////////////////////////////////////////////////////////////////////////////////////

void tulis_sd_card(){
  float h = dht.readHumidity();
  float t = dht.readTemperature();  
//  DateTime now = rtc.now();
//  String date_rtc = String(now.year(),DEC) + "-" + String(now.month(), DEC) + "-" + String(now.day(), DEC);
//  String time_rtc = String(now.hour(), DEC) + ":" + String(now.minute(), DEC) + ":" + String(now.second(), DEC);
  DateTime now = rtc.now();
  String year_now = String(now.year(),DEC); 
  String month_now = "";
  int month_dtg =now.month();
  if(month_dtg < 10){
    month_now += '0';
  }
  month_now += month_dtg;  
  String day_now = "";
  int day_dtg =now.day();
  if(day_dtg < 10){
    day_now += '0';
  }
  day_now += day_dtg; 
  String hour_now = "";
  int hour_dtg =now.hour();
  if(hour_dtg < 10){
    hour_now += '0';
  }
  hour_now += hour_dtg;
  String minute_now = "";
  int minute_dtg =now.minute();
  if(minute_dtg < 10){
    minute_now += '0';
  }
  minute_now += minute_dtg;
  String second_now = "";
  int second_dtg =now.second();
  if(second_dtg < 10){
    second_now += '0';
  }
  second_now += second_dtg;
  String date_rtc = String(now.year(),DEC) + "-" + month_now + "-" + day_now;
  String time_rtc = hour_now + ":" + minute_now + ":" + second_now;   
  String data = String(t+temp_call)+"@"+String(h+rh_call)+"@"+String(date_rtc)+"@"+String(time_rtc);
  
  myFile = SD.open("log.txt", FILE_WRITE);
  if (myFile) {  
    Serial.print("Writing to log.txt..."); 
    myFile.println(data);
    myFile.close();
    Serial.println("save sd = "+data);
    
    
  } else {
    u8g2.clearBuffer();
    u8g2.clearBuffer();
    u8g2.setFont(u8g2_font_logisoso20_tr); // choose a suitable font 42 pixel
    u8g2.setCursor(2,52);
    u8g2.print("log error....");
    u8g2.sendBuffer(); 
    delay(1000);
    ESP.restart();    
  } 

  myFile = SD.open("log.txt");
  if (myFile) {
    Serial.println("log.txt:");

    // read from the file until there's nothing else in it:
    while (myFile.available()) {
      Serial.write(myFile.read());
    }
    // close the file:
    myFile.close();
  } else {
    // if the file didn't open, print an error:
    Serial.println("error opening log.txt");
  }
  
}
////////////////////////////////////////////////////////////////////////////////////////////////
void cek_data_sdcard_and_send_to_firebase(){
  
  String pemilik_ping = httpPOSTRequest_pemilik();
  JSONVar var_pemilik_ping = JSON.parse(pemilik_ping);
  pemilik_ping = var_pemilik_ping["pemilik"];

   
  if (pemilik_ping !=""){
    
    if(SD.exists("log.txt")){
      myFile = SD.open("log.txt");
      String data;
      int count =1;
      if(myFile){    
        while(myFile.available()){
          data = String(myFile.readStringUntil('\n'));
          data.trim();
          Serial.println(data);
          httpPOSTRequest_post_data(data);    
          delay(3000);      
        }
        
        myFile.close();
        SD.remove("log.txt"); 
        Serial.println("remove log.txt");    
        
      }else{
        u8g2.clearBuffer();
        u8g2.clearBuffer();
        u8g2.setFont(u8g2_font_logisoso20_tr); // choose a suitable font 42 pixel
        u8g2.setCursor(2,52);
        u8g2.print("log error....");
        u8g2.sendBuffer(); 
        delay(1000);
        ESP.restart();    
      }   
    }else{
    } 
    
  }else{
    Serial.println("ping pemilik gagal cek data sd card");
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
//////////////////////////////////////////////////////////////////////////////////////
void update_firmware(){ 
  u8g2.clearBuffer();          // clear the internal memory
  u8g2.setFont(u8g2_font_logisoso20_tr);
  u8g2.setCursor(2,30);
  u8g2.print("update..");
  u8g2.setCursor(2,62);
  u8g2.print("firmware");
  u8g2.sendBuffer(); 
  delay(1000);
  WiFiClient client;
  ESPhttpUpdate.setLedPin(LED_BUILTIN, LOW);
  t_httpUpdate_return ret = ESPhttpUpdate.update(URL_update,"",Fingerprint);
  switch (ret) {
    case HTTP_UPDATE_FAILED:       
        Serial.printf("HTTP_UPDATE_FAILD Error (%d): %s\n", ESPhttpUpdate.getLastError(), ESPhttpUpdate.getLastErrorString().c_str());
        ESP.restart(); 
        break;
    case HTTP_UPDATE_NO_UPDATES:      
      Serial.println("HTTP_UPDATE_NO_UPDATES");   
      break;
    case HTTP_UPDATE_OK:   
      Serial.println("HTTP_UPDATE_OK");    
      break;
  }
  yield();
}
String get_version_firmware(){
  HTTPClient http;
  http.begin(get_versionfirmware);
  http.addHeader("Content-Type", "application/json");
  int body = http.POST("{\n\t\"id\":\"" + id_device + "\"\n}");
  String payload = http.getString();
  
//  Serial.print("HTTP Response version firmware: ");
//  Serial.println(body);
  
//  Serial.print("HTTP Response data delay: ");
//  Serial.println(payload);  
  if (body==200){
    http.end();
    return payload;
  }
  else {
    String error = "error request"; 
    http.end();  
  }
  http.end();
}
////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////
void loop() {
  u8g2.clearBuffer();
  if(Thread1.shouldRun())
    Thread1.run();
  if(Thread2.shouldRun())
    Thread2.run();
  if(Thread3.shouldRun())
    Thread3.run();
  if(Thread4.shouldRun())
    Thread4.run(); 
  int x = 0;
  x = 1 + 2;
}
///////////////////////////////////////////////////////////////////////////////////////////////////
