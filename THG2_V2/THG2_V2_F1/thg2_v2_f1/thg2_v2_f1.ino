////////////////////////
////author by Putra////
/////////THG2//////////
////////////////////////////////////////////////////////////////////////////////////////////////////
void(* service_reset) (void) = 0;
////////////////////////////////////////////////////////////////////////////////////////////////////
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
#include <OneWire.h>
#include <DallasTemperature.h>
#define ONE_WIRE_BUS 0
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensorSuhu(&oneWire);
float suhuSekarang;
//#include "DHTesp.h"
//#include <SPI.h>
#include <SD.h>
//DHTesp dht;
///////////////////////////////////
//#include "DHT.h"
//#define DHTPIN 0
//#define DHTTYPE DHT22
//DHT dht(DHTPIN, DHTTYPE);
///////////////////////////////////
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
#define URL_update "https://raw.githubusercontent.com/PutraNasri/update_firmware_thg2_v1/main/THG2_V2/ini.bin"
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
float rh_call = 0;
float temp_call = 0;
int menit_to_detik = 60;
int detik_to_milidetik = 1000;
const int pin_sensor = 0;

const int btn_acpn = 16;

const int pin_led_acpn = 1;    
const int pin_led_no_ok = 2;   
const int pin_led_ok = 3;  
    
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
Thread Thread5 = Thread();

////////////////////////////////////////////////////////////////////////////////////////////////////
void setup() {
 
  Serial.begin(115200);
  
  pinMode(btn_acpn,INPUT);
  pinMode(pin_sensor,INPUT);
  
  pinMode(pin_led_acpn,OUTPUT);
  pinMode(pin_led_no_ok,OUTPUT);
  pinMode(pin_led_ok,OUTPUT);
  
  delay(1000);

  sensorSuhu.begin();
  u8g2.begin();
//  dht.begin();
  u8g2.clearBuffer();
  u8g2.clearBuffer();
  u8g2.clearBuffer();

////////////////////////////////////////////////////////////////////////////////////////
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }
  
//  Serial.print("Initializing SD card...");

  if (!SD.begin(15)) {
    Serial.println("initialization failed!");
    while (1);
  }
//  Serial.println("initialization done.");
  
  if(SD.exists("id_device.txt")){
    myFile = SD.open("id_device.txt");
    String id_device_sdcard;
    int count=1;
    if(myFile){
//      Serial.println("id_device.txt:");
      while(myFile.available()){
        id_device_sdcard = String(myFile.readStringUntil('\n'));
        id_device_sdcard.trim();
//        Serial.println("ini id device di sd card = "+id_device_sdcard);
        
        id_device = id_device_sdcard;
        ssid      = id_device.c_str();

        
        delay(3000);  
      }
      myFile.close();
    }else{
//      Serial.println("error opening test.txt");
    }
  }else{
//    Serial.println("data id_device.txt di sdcard kosong");
  }
//  SD.remove("test.txt");    
 ////////////////////////////////////////////////////////////////////////////////////////
//  Serial.print("Connecting to ");
  WiFiManager wifiManager;
  wifiManager.setTimeout(60);
  wifiManager.setBreakAfterConfig(true);
  digitalWrite(pin_led_no_ok,LOW);
  digitalWrite(pin_led_ok,LOW);
  digitalWrite(pin_led_acpn,HIGH);
  if (!wifiManager.autoConnect(ssid, password)) {
//    Serial.println("failed to connect, we should reset as see if it connects");
    delay(3000);
    ESP.reset();
    delay(5000);
  }
  
  Serial.println("");
  Serial.println("WiFi connected.");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  digitalWrite(pin_led_no_ok,HIGH);
  digitalWrite(pin_led_ok,LOW);
  digitalWrite(pin_led_acpn,LOW);
  
  timeClient.begin();
  timeClient.setTimeOffset(25200);
  
  
//////////////////////////////////////////////////////////////////////////////////////////
  delay(1000);
  String delay_server_firebase = httpPOSTRequest_delay();
  JSONVar var_delay_firebase = JSON.parse(delay_server_firebase);
  delay_server_firebase = var_delay_firebase["delay"];
//  Serial.println("delay firebase = "+delay_server_firebase);
  
  delay(1000);
  String sts_adjustment_rh_temp_firebase = httpPOSTRequest_adjustment_rh_temp();
  JSONVar var_adjustment_rh_temp_firebase =JSON.parse(sts_adjustment_rh_temp_firebase);
  sts_adjustment_rh_temp_firebase = var_adjustment_rh_temp_firebase["adjustment_rh_temp"];
//  Serial.println("adjustment firebase = "+sts_adjustment_rh_temp_firebase);


  if (delay_server_firebase == ""){
    delay_server_firebase = "2";
  }
  else if (sts_adjustment_rh_temp_firebase == ""){
    sts_adjustment_rh_temp_firebase="0@0";
  }

  delay_server = delay_server_firebase; 
//  Serial.println(delay_server);
  sts_adjustment_rh_temp = sts_adjustment_rh_temp_firebase;
//  Serial.println(sts_adjustment_rh_temp);
  
  char *sts_adjustment_rh_tempp = strdup(sts_adjustment_rh_temp.c_str());
  char * rh = strtok(sts_adjustment_rh_tempp,"@");
  char * temp = strtok(NULL,"@");
//  rh_call=atoi(rh);
//  temp_call=atoi(temp);
  rh_call=atof(rh);
  temp_call=atof(temp);

  
///////////////////////////////////////////////////////////////////////////////////////////
  if (delay_server != ""){
    sts_server = "1";
  }
  int delay_server_service = delay_server.toInt();
  Thread1.onRun(service);
  Thread1.setInterval(delay_server_service*menit_to_detik*detik_to_milidetik);
  Thread2.onRun(acpn_mode);
  Thread2.setInterval(500);
  Thread3.onRun(led_conf);
  Thread3.setInterval(500);
  Thread4.onRun(service_control);
  Thread4.setInterval(30000);
//  Thread4.setInterval(50000);
  Thread5.onRun(service_lcd);
  Thread5.setInterval(2000);
//  Serial.println("device ready "+version_firmware);

 
}
////////////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////////////////

void service(){
  if(WiFi.status() == WL_CONNECTED){  
    if(pinger.Ping("otoridashboard.id") == false){
//      Serial.println("Error during ping command service.");
      sts_server = "0";
      tulis_sd_card(); 
      //lakukan cek ping jika ada lanjut get api jika tidak, simpan data to sd card\
      
    }else{
//      Serial.println("connected.");
      String pemilik = httpPOSTRequest_pemilik();
      JSONVar var_pemilik = JSON.parse(pemilik);
      pemilik = var_pemilik["pemilik"];
//      Serial.println(pemilik);

      if(pemilik != ""){
        if(pemilik != "no_id_user" && pemilik != "lock"){
          sts_server = "1";
          cek_data_sdcard_and_send_to_firebase();
          
//          float h = dht.readHumidity();
//          float t = dht.readTemperature();
          float temperature = getTemp();
          float h = 00;
          
          while(!timeClient.update()) {
            timeClient.forceUpdate();
          }
          formattedDate = timeClient.getFormattedDate();
          int splitT = formattedDate.indexOf("T");
          dayStamp = formattedDate.substring(0, splitT);
          timeStamp = formattedDate.substring(splitT+1, formattedDate.length()-1);
          String data = String(temperature+temp_call)+"@"+String(h+rh_call)+"@"+String(dayStamp)+"@"+String(timeStamp);
          httpPOSTRequest_post_data(data);  
        }else if(pemilik=="no_id_user"){
          sts_server = "0";
          Serial.println("no_id_user");
        }else if(pemilik=="lock"){
          sts_server = "0";
          Serial.println("device lock");
        }
//        else{
//          sts_server = "0"; 
//          Serial.println("error lain");
//        }
           
      }else{
        Serial.println("data pemilik firebase kosong");
//        tulis_sd_card(); 
      }  
    }
    yield();
  } 
  
  
//  ///////////////////////wifi can't connect//////////////////////////////////////////
  else {
//    Serial.println("wifi not connected");
    sts_server = "0";  
//    tulis_sd_card(); //tulis di sdcard   
  }

  yield();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void acpn_mode(){
  if(digitalRead(btn_acpn)==HIGH){
      u8g2.clearBuffer();          // clear the internal memory
      u8g2.setFont(u8g2_font_logisoso42_tr); // choose a suitable font 42 pixel
      u8g2.setCursor(2,42);
      u8g2.print("REST");
      u8g2.sendBuffer(); 
      WiFiManager wifiManager;
//      Serial.println("ACPN MODE");
      
      digitalWrite(pin_led_no_ok,LOW);
      digitalWrite(pin_led_ok,LOW);
      digitalWrite(pin_led_acpn,HIGH);
      
      wifiManager.resetSettings();
      wifiManager.autoConnect(ssid, password);
      wifiManager.setBreakAfterConfig(true);   
  }
  yield();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void led_conf(){
  if(sts_server == "0"){
    digitalWrite(pin_led_no_ok,HIGH);
    digitalWrite(pin_led_ok,LOW);
    digitalWrite(pin_led_acpn,LOW);
  }
  else if (sts_server == "1"){
    digitalWrite(pin_led_no_ok,LOW);
    digitalWrite(pin_led_ok,HIGH);
    digitalWrite(pin_led_acpn,LOW);
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
//      Serial.println("Error during ping command service_control.");
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
      Serial.println("versi firmware control = "+firmware_version_control);
      delay(1000);

      
      String fingerprint_control = get_version_firmware();
      JSONVar var_fingerprint = JSON.parse(fingerprint_control);
      fingerprint_control = var_fingerprint["fingerprint"];
//      Serial.println("versi fingerprint control = "+fingerprint_control);
      
      Fingerprint=fingerprint_control;

      if(delay_server_control !="" && sts_adjustment_rh_temp_control !="" && firmware_version_control !="" && fingerprint_control !=""){
          if (delay_server != delay_server_control){
//            Serial.println("reset by delay");
            service_reset();
          }else if(sts_adjustment_rh_temp != sts_adjustment_rh_temp_control){
//            Serial.println("reset by adjust");
            service_reset();
          }else if(firmware_version_control != version_firmware){
//            Serial.println("update firmware");
            update_firmware();
          }else{
//            Serial.println("passs data ada");
          }
      }else{
//        Serial.println("passss data kosong");
//        Serial.println("delay_server_control = "+delay_server_control+"sts_adjustment_rh_temp_control = "+sts_adjustment_rh_temp_control+"fingerprint_control = "+fingerprint_control);
      }
      
    }  
    yield();

  }else if (WiFi.status() != WL_CONNECTED){
//    Serial.println("error wifi");
  }

  yield();
  
}


////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////
void service_lcd(){
  
  String tempp = String(getTemp()+temp_call);
  char * temp = strdup(tempp.c_str());
//  Serial.println(tempp);
  
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
////////////////////////////////////////////////////////////////////////////////////////////////
String httpPOSTRequest_pemilik(){
  HTTPClient http;
  http.begin(get_pemilik);
  http.addHeader("Content-Type", "application/json");
  int body = http.POST("{\n\t\"id\":\"" + id_device + "\"\n}");
  String payload = http.getString();
//  Serial.print("HTTP Response code pemilik: ");
//  Serial.println(body);
//  Serial.print("HTTP Response data pemilik: ");
//  Serial.println(payload);
  http.end();

  if (body==200){
    return payload;
  }
  else{
    return String(body);
  }

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
  http.end();

  if (body==200){
    return payload;
  }
  else{
    return String(body);
  }
  
 
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
  http.end();

  if (body==200){
    return payload;
  }
  else{
    return String(body);
  }
  
//  return payload;
 
}
////////////////////////////////////////////////////////////////////////////////////////////////
void httpPOSTRequest_post_data(String data){
  HTTPClient http;
  http.begin(post_data);
  http.addHeader("Content-Type", "application/json");
//  String params = "{\n\t\"id\":\""+id_device+"\",\n\t\"data\":\""+data+"\"\n}";

  String params = "{\"id\":\""+id_device+"\",\"data\":\""+String(data)+"\"}";
                  
//  String params = "{\"id\":\""+id_device+"\",\"data\":\""+data+"\"}";
//  Serial.println(params);
  int body = http.POST(params);
  String payload = http.getString();
//  Serial.print("HTTP Response code post data: ");
//  Serial.println(body);
//  Serial.print("HTTP Response data post data: ");
//  Serial.println(payload);
  http.end();
  yield();
 
}
///////////////////////////////////////////////////////////////
void tulis_sd_card(){
  
  float h = 00;
  float temperature = getTemp()+00;
  
  while(!timeClient.update()) {
    timeClient.forceUpdate();
  }
  formattedDate = timeClient.getFormattedDate();
  int splitT = formattedDate.indexOf("T");
  dayStamp = formattedDate.substring(0, splitT);
  timeStamp = formattedDate.substring(splitT+1, formattedDate.length()-1);
  String data = String(temperature+temp_call)+"@"+String(h+rh_call)+"@"+String(dayStamp)+"@"+String(timeStamp);
  
  //////////////////////////////////////////////////////////////////////////
//  Serial.println("menulis sdcard");
  myFile = SD.open("test.txt", FILE_WRITE);

  // if the file opened okay, write to it:
  if (myFile) {
    Serial.print("Writing to test.txt...");
    ///////////////////////////////////////////////////////////////////////////
//    float h = dht.readHumidity();
//    float t = dht.readTemperature();
//    while(!timeClient.update()) {
//      timeClient.forceUpdate();
//    }
//    formattedDate = timeClient.getFormattedDate();
//    int splitT = formattedDate.indexOf("T");
//    dayStamp = formattedDate.substring(0, splitT);
//    timeStamp = formattedDate.substring(splitT+1, formattedDate.length()-1);
//    String data = String(t+temp_call)+"@"+String(h+rh_call)+"@"+String(dayStamp)+"@"+String(timeStamp);
//    //////////////////////////////////////////////////////////////////////////
    myFile.println(data);
    // close the file:
    myFile.close();
    Serial.println("done.");
  } else {
    // if the file didn't open, print an error:
    Serial.println("error opening test.txt");
  }

  
}
////////////////////////////////////////////////////////////////////////////////////////////////
void cek_data_sdcard_and_send_to_firebase(){
  if(SD.exists("test.txt")){
    myFile = SD.open("test.txt");
    String data;
    int count =1;
    if(myFile){
      Serial.println("test.txt:");
      while(myFile.available()){
        data = String(myFile.readStringUntil('\n'));
        data.trim();
        httpPOSTRequest_post_data(data);
//        Serial.println("ini datasd = "+data);
        delay(3000);      
      }
      myFile.close(); 
      SD.remove("test.txt");    
    }else{
//      Serial.println("error opening test.txt");
    }   
  }else{
//    Serial.println("data sd kosong");
    //lanjut pass
  }
 
}
////////////////////////////////////////////////////////////////////////////////////////////////
void update_firmware(){
//  Serial.println("Download and flash new firmware.....");
  
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
//  Serial.print("HTTP Response code delay: ");
//  Serial.println(body);
//  Serial.print("HTTP Response data delay: ");
//  Serial.println(payload);
  http.end();
  
  if (body==200){
    return payload;
  }
  else{
    return String(body);
  }
}
////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////
float getTemp(){
  sensorSuhu.requestTemperatures();
  float suhu = sensorSuhu.getTempCByIndex(0);
  return suhu;

}
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
  if(Thread5.shouldRun())
    Thread5.run();
  
  int x = 0;
  x = 1 + 2;
}
///////////////////////////////////////////////////////////////////////////////////////////////////
