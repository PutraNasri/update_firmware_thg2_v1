///////////////////////
////author by Putra////
/////////THG2//////////
////////////////////////////////////////////////////////////////////////////////////////////////////
void(* service_reset) (void) = 0;
////////////////////////////////////////////////////////////////////////////////////////////////////

// Date and time functions using a DS1307 RTC connected via I2C and Wire lib
#include <Thread.h>
#include <Pinger.h>
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
//////////////////////////////////
String get_pemilik = "http://otoridashboard.id/thg2/get_pemilik";
String get_adjustment_rh_temp = "http://otoridashboard.id/thg2/get_adjustment_rh_temp";
String get_delay = "http://otoridashboard.id/thg2/get_delay";
String get_versionfirmware = "http://otoridashboard.id/thg2/versi";
String post_data = "http://otoridashboard.id/thg2/nulis_data";
String Fingerprint = "null";
#define URL_update "https://raw.githubusercontent.com/PutraNasri/update_firmware_thg2_v1/main/THG2_V1/ini.bin"
HTTPClient http;
/////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////bagian yang harus di sesuaikan////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////
String id_device      = "XXXXXXXX";
const char* ssid      = "XXXXXXXX";
String version_firmware = "F1";
/////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////
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
//const int pin_led_acpn = 1;    
//const int pin_led_no_ok = 2;   
//const int pin_led_ok = 3;  


WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP);
File myFile;
String formattedDate;
String dayStamp;
String timeStamp;
extern "C"
{
  #include <lwip/icmp.h> 
}

Pinger pinger;


Thread Thread1 = Thread();
Thread Thread2 = Thread();
Thread Thread3 = Thread();
Thread Thread4 = Thread();


#if defined(ARDUINO_ARCH_SAMD)
// for Zero, output on USB Serial console, remove line below if using programming port to program the Zero!
   #define Serial SerialUSB
#endif
RTC_DS1307 rtc;
char daysOfTheWeek[7][12] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};




void setup () {

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

  if (! rtc.begin()) {
    Serial.println("Couldn't find RTC");
    while (1);
  }

  if (! rtc.isrunning()) {
    Serial.println("RTC is NOT running!");
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
  
  Serial.print("Initializing SD card...");

  if (!SD.begin(15)) {
    Serial.println("initialization failed!");
    while (1);
  }
  Serial.println("initialization done.");
  delay(1000);
////////////////////////////////////////////////////////////////////////////////////
  if(SD.exists("id_device.txt")){
    myFile = SD.open("id_device.txt");
    String id_device_sdcard;
    int count=1;
    if(myFile){
      Serial.println("id_device.txt:");
      while(myFile.available()){
        id_device_sdcard = String(myFile.readStringUntil('\n'));
        id_device_sdcard.trim();
        Serial.println("ini id device di sd card = "+id_device_sdcard);
        
        id_device = id_device_sdcard;
        ssid      = id_device.c_str();

        
        delay(3000);  
      }
      myFile.close();
    }else{
      Serial.println("error opening test.txt");
    }
  }else{
    Serial.println("data id_device.txt di sdcard kosong");
  }
/////////////////////////////////////////////////////////////////////  
  Serial.print("Connecting to ");
  WiFiManager wifiManager;
  wifiManager.setTimeout(60);
  wifiManager.setBreakAfterConfig(true);
  if (!wifiManager.autoConnect(ssid, password)) {
    Serial.println("failed to connect, we should reset as see if it connects");
//    delay(3000);
//    ESP.reset();
//    delay(5000);
    Serial.println("pass wifimanager");
  }
////////////////////////////////////////////////////////////////////////  
  timeClient.begin();
  timeClient.setTimeOffset(25200);
////////////////////////////////////////////////////////////////
  if(SD.exists("config.txt")){
    Serial.println("file sudah ada");
    //ambil data nya sesuaikan dengan variabel
    myFile = SD.open("config.txt");
    String data_config;
    int count=1;
    String id_pemilik_config,delay_config,adjustment_config,version_config;
    if(myFile){
      Serial.println("read config.txt: \n");
      int n = 0;
      while(myFile.available()){


        data_config = String(myFile.readStringUntil('\n'));
//        data_config = String(myFile.readString());
        data_config.trim();
//        Serial.println(data_config);
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
      Serial.println(id_pemilik_config);
      Serial.println(delay_config);
      Serial.println(adjustment_config);
      Serial.println(version_config);
      
      
      sts_adjustment_rh_temp = adjustment_config;
      delay_server = delay_config;
      pemilik = id_pemilik_config;
      
    }else{
      Serial.println("error opening config.txt");
      //tampilkan error config di lcd lalu restart
    }
    
  }else{ //jika file config tidak tersedia
    if(WiFi.status() == WL_CONNECTED){
      Serial.println("wifi connected");

      String pemilik_ping = httpPOSTRequest_pemilik();
      JSONVar var_pemilik_ping = JSON.parse(pemilik_ping);
      pemilik_ping = var_pemilik_ping["pemilik"];
      Serial.println("pemilik_ping = "+pemilik_ping);
      
      if(pemilik_ping ==""){  
        Serial.println("Error during ping command service.");
        //tampilkan jaringan tidak stabil di lcd dan restart  
        u8g2.clearBuffer();
        u8g2.setFont(u8g2_font_logisoso20_tr); // choose a suitable font 42 pixel
        u8g2.setCursor(2,52);
        u8g2.print("error ping...");
        u8g2.sendBuffer();
        delay (3000) ;
        service_reset();
             
      }else{
        Serial.println("connected to server.");
        // get api lalu tulis ke sd card
        timeClient.begin();
        timeClient.setTimeOffset(25200);
        
        while(!timeClient.update()) {
            timeClient.forceUpdate();
        }
        formattedDate = timeClient.getFormattedDate();
        int splitT = formattedDate.indexOf("T");
        dayStamp = formattedDate.substring(0, splitT);
        timeStamp = formattedDate.substring(splitT+1, formattedDate.length()-1);
        Serial.println(dayStamp+" "+timeStamp);
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

        String pemilik = httpPOSTRequest_pemilik();
        JSONVar var_pemilik = JSON.parse(pemilik);
        pemilik = var_pemilik["pemilik"];
        Serial.println("pemilik = "+pemilik);
        delay(1000);
        String delay_server_control = httpPOSTRequest_delay();
        JSONVar var_delay_control = JSON.parse(delay_server_control);
        delay_server_control = var_delay_control["delay"];
        Serial.println("delay server = "+delay_server_control);
        delay(1000);
        String sts_adjustment_rh_temp_control = httpPOSTRequest_adjustment_rh_temp();
        JSONVar var_adjustment_rh_temp_control =JSON.parse(sts_adjustment_rh_temp_control);
        sts_adjustment_rh_temp_control = var_adjustment_rh_temp_control["adjustment_rh_temp"];
        Serial.println("adjustment server = "+sts_adjustment_rh_temp_control);
        delay(1000);
        String firmware_version_control = get_version_firmware();
        JSONVar var_firmware_version = JSON.parse(firmware_version_control);
        firmware_version_control = var_firmware_version["versi_firmware"];
        Serial.println("versi firmware server = "+firmware_version_control);
        delay(1000);

      
        Serial.println("menulis config to sdcard");
        
        if(pemilik == ""){
          service_reset();
        }
        if(delay_server_control == ""){
          service_reset();  
        }
        if(sts_adjustment_rh_temp_control == ""){
          service_reset();
        }
        if(firmware_version_control == ""){
          service_reset();
        }
        
        myFile = SD.open("config.txt", FILE_WRITE);
        if(myFile){
          Serial.print("Writing to id_device.txt...");
          myFile.println(pemilik);
          myFile.println(delay_server_control);
          myFile.println(sts_adjustment_rh_temp_control);
          myFile.println(firmware_version_control);
          myFile.close();
          Serial.println("done set config.");   
          service_reset(); 
        }else{
          Serial.println("error opening test.txt");
          //tampilkan error config di lcd lalu restart
        } 
      }
    }else{
      Serial.println("wifi not connected");
      //tampilkan need wifi di lcd dan restart
    }   
  }

  char *sts_adjustment_rh_tempp = strdup(sts_adjustment_rh_temp.c_str());
  char * rh = strtok(sts_adjustment_rh_tempp,"@");
  char * temp = strtok(NULL,"@");
//  rh_call=atoi(rh);
//  temp_call=atoi(temp);
  rh_call=atof(rh);
  temp_call=atof(temp);

  Serial.println (rh_call);
  Serial.println (temp_call);

  int delay_server_service = delay_server.toInt();
  Thread1.onRun(service);
  Thread1.setInterval(delay_server_service*menit_to_detik*detik_to_milidetik);
  Thread2.onRun(acpn_mode);
  Thread2.setInterval(500);
  Thread3.onRun(service_control);
  Thread3.setInterval(30000);
  Thread4.onRun(service_lcd);
  Thread4.setInterval(2000);
  Serial.println("device ready "+version_firmware);
  
}

void service(){
  if(WiFi.status() == WL_CONNECTED){  
    
    String pemilik_ping = httpPOSTRequest_pemilik();
    JSONVar var_pemilik_ping = JSON.parse(pemilik_ping);
    pemilik_ping = var_pemilik_ping["pemilik"];
    Serial.println("pemilik_ping = "+pemilik_ping);

    if (pemilik_ping ==""){
        tulis_sd_card();
        //tulis ke sd card
    }else{
      Serial.println("connected.");
      
      Serial.println(pemilik);

      if(pemilik != ""){
        if(pemilik != "no_id_user" && pemilik != "lock"){
          sts_server = "1";
          cek_data_sdcard_and_send_to_firebase();
          float h = dht.readHumidity();
          float t = dht.readTemperature();
          while(!timeClient.update()) {
            timeClient.forceUpdate();
          }
          formattedDate = timeClient.getFormattedDate();
          int splitT = formattedDate.indexOf("T");
          dayStamp = formattedDate.substring(0, splitT);
          timeStamp = formattedDate.substring(splitT+1, formattedDate.length()-1);
          String data = String(t+temp_call)+"@"+String(h+rh_call)+"@"+String(dayStamp)+"@"+String(timeStamp);
          httpPOSTRequest_post_data(data);  
        }else if(pemilik=="no_id_user"){
          Serial.println("no_id_user");
        }else if(pemilik=="lock"){
          Serial.println("device lock");
        }         
      }else{
        Serial.println("data pemilik firebase kosong");
      }  
    }
    
  }else{
    Serial.println("wifi not connected");
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
      Serial.println("ACPN MODE");
      
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

    //lakukan cek ping jika ada lanjut get api jika tidak pass
    //get api apabila rspon code selain 200 pass
    delay(1000);
    if(pinger.Ping("otoridashboard.id") == false){
      Serial.println("Error during ping command service_control.");
      sts_server = "0"; 
    }else{
      Serial.println("cek service control");
      String delay_server_control = httpPOSTRequest_delay();
      JSONVar var_delay_control = JSON.parse(delay_server_control);
      delay_server_control = var_delay_control["delay"];
//      Serial.println("delay control = "+delay_server);
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

      if(delay_server_control !="" && sts_adjustment_rh_temp_control !="" && firmware_version_control !="" && fingerprint_control !=""){
          if (delay_server != delay_server_control){
            Serial.println("reset by delay");
            SD.remove("config.txt");  
            delay(1000);  
            service_reset();
          }else if(sts_adjustment_rh_temp != sts_adjustment_rh_temp_control){
            Serial.println("reset by adjust");
            SD.remove("config.txt");  
            delay(1000);
            service_reset();
          }else if(firmware_version_control != version_firmware){
            Serial.println("update firmware");
            SD.remove("config.txt");  
            delay(1000);
            update_firmware();
          }else{
            Serial.println("passs data ada");
            Serial.println("delay = "+delay_server);
            Serial.println("adjust = "+sts_adjustment_rh_temp);
            Serial.println("version = "+version_firmware);
            Serial.println("pemilik = "+pemilik);
          }
      }else{
        Serial.println("delay_server_control = "+delay_server_control+"sts_adjustment_rh_temp_control = "+sts_adjustment_rh_temp_control+"fingerprint_control = "+fingerprint_control);
      }
      
    }  
    yield();

  }else if (WiFi.status() != WL_CONNECTED){
    Serial.println("error wifi pass");
  }

  yield();
  
}


////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////
void service_lcd(){
  
  String tempp = String(dht.readTemperature()+temp_call);
  char * temp = strdup(tempp.c_str());
  String humm =String(dht.readHumidity()+rh_call);
  char * hum = strdup(humm.c_str());
  
  if (tempp == "nan"){
    service_reset();
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
    u8g2.print(belakang_temp);   // nilai belakang koma  
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
  HTTPClient http;
  http.begin(get_pemilik);
  http.addHeader("Content-Type", "application/json");
  int body = http.POST("{\n\t\"id\":\"" + id_device + "\"\n}");
  String payload = http.getString();
  Serial.print("HTTP Response code pemilik: ");
  Serial.println(body);
//  Serial.print("HTTP Response data pemilik: ");
//  Serial.println(payload);
  

  if (body==200){
    return payload;
  }
  else {
    String error = "error request";
    Serial.println("error request pemilik");
    return String(error);
    
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
  Serial.print("HTTP Response code delay: ");
  Serial.println(body);
//  Serial.print("HTTP Response data delay: ");
//  Serial.println(payload);
 

  if (body==200){
    return payload;
  }
  else {
    String error = "error request";
    Serial.println("error request delay");
    return String(error);
    
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
  Serial.print("HTTP Response code adjustment: ");
  Serial.println(body);
//  Serial.print("HTTP Response data adjustment: ");
//  Serial.println(payload);
  
  if (body==200){
    return payload;
  }
  else {
    String error = "error request";
    Serial.println("error request adjust");
    return String(error);
    
  }
  http.end();
  
//  return payload;
 
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
  Serial.print("HTTP Response code post data: ");
  Serial.println(body);
//  Serial.print("HTTP Response data post data: ");
//  Serial.println(payload);
 
 
  
  
  if (body==200){
    return payload;
  }
  else {
    String error = "error request";
    Serial.println("error");
    tulis_sd_card();
    return String(error);
    
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
  Serial.println ("simpan data = "+data);
  //////////////////////////////////////////////////////////////////////////
  Serial.println("menulis sdcard");
  myFile = SD.open("log.txt", FILE_WRITE);

  if (myFile) {
    Serial.print("Writing to log.txt...");
    myFile.println(data);
    myFile.close();
    Serial.println("done.");
  } else {
    Serial.println("error opening log.txt");
  }

  
}
////////////////////////////////////////////////////////////////////////////////////////////////
void cek_data_sdcard_and_send_to_firebase(){
  if(SD.exists("log.txt")){
    myFile = SD.open("log.txt");
    String data;
    int count =1;
    if(myFile){
      Serial.println("file log.txt ada ");
      while(myFile.available()){
        data = String(myFile.readStringUntil('\n'));
        data.trim();
        httpPOSTRequest_post_data(data);
        Serial.println("ini datasd = "+data);
        delay(3000);      
      }
      myFile.close(); 
      SD.remove("log.txt");    
    }else{
      Serial.println("error opening test.txt");
    }   
  }else{
    Serial.println("data sd kosong");
    //lanjut pass
  }
 
}
////////////////////////////////////////////////////////////////////////////////////////////////

void update_firmware(){
  Serial.println("Download and flash new firmware.....");
  
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
        service_reset();
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
  Serial.print("HTTP Response version firmware: ");
  Serial.println(body);
//  Serial.print("HTTP Response data delay: ");
//  Serial.println(payload);
  
  
  if (body==200){
    return payload;
  }
  else {
    String error = "error request";
    Serial.println("error request firmware");
    return String(error);
    
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
