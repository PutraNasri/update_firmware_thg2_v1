////////////////////////
////author by Putra////
/////////THG1//////////
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
#include <WiFiClient.h>
#include <Arduino_JSON.h>
#include <WiFiManager.h>
#include "DHTesp.h"
//DHTesp dht;
///////////////////////////////////
#include "DHT.h"
#define DHTPIN 5
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);
///////////////////////////////////
String get_pemilik = "http://otoridashboard.id/get_pemilik";
String get_adjustment_rh_temp = "http://otoridashboard.id/get_adjustment_rh_temp";
String get_delay = "http://otoridashboard.id/get_delay";
String post_data = "http://otoridashboard.id/nulis_data";
/////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////bagian yang harus di sesuaikan////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////
String id_device      = "xxx";
const char* ssid      = "xxx";
String version_firmware = "F1";
/////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////
const char* password = "12345678";
String sts_server = "0";
String sts_adjustment_rh_temp = "0@0";
String delay_server = "1";
int rh_call = 0;
int temp_call = 0;
int menit_to_detik = 60;
int detik_to_milidetik = 1000;
const int pin_sensor = 5;

const int btn_acpn = 12;
const int pin_led_acpn = 13;    
const int pin_led_no_ok = 14;   
const int pin_led_ok = 15;      
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP);
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
//Thread Thread5 = Thread();
////////////////////////////////////////////////////////////////////////////////////////////////////
void setup() {
  Serial.begin(115200);
  Serial.println("ID_DEVICE = "+id_device);
  pinMode(btn_acpn,INPUT);
  pinMode(pin_sensor,INPUT);
  pinMode(pin_led_acpn,OUTPUT);
  pinMode(pin_led_no_ok,OUTPUT);
  pinMode(pin_led_ok,OUTPUT);
  dht.begin();
  Serial.print("Connecting to ");
  WiFiManager wifiManager;
  wifiManager.setTimeout(60);
  wifiManager.setBreakAfterConfig(true);
  digitalWrite(pin_led_no_ok,LOW);
  digitalWrite(pin_led_ok,LOW);
  digitalWrite(pin_led_acpn,HIGH);
  if (!wifiManager.autoConnect(ssid, password)) {
    Serial.println("failed to connect, we should reset as see if it connects");
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
  
  /////////////////////////////////////////////////////////////////////////
  delay(1000);
  String delay_server_firebase = httpPOSTRequest_delay();
  JSONVar var_delay_firebase = JSON.parse(delay_server_firebase);
  delay_server_firebase = var_delay_firebase["delay"];
  Serial.println("delay firebase = "+delay_server_firebase);
  
  delay(1000);
  String sts_adjustment_rh_temp_firebase = httpPOSTRequest_adjustment_rh_temp();
  JSONVar var_adjustment_rh_temp_firebase =JSON.parse(sts_adjustment_rh_temp_firebase);
  sts_adjustment_rh_temp_firebase = var_adjustment_rh_temp_firebase["adjustment_rh_temp"];
  Serial.println("adjustment firebase = "+sts_adjustment_rh_temp_firebase);

  if (delay_server_firebase ==""){
    service_reset();
  }else if(sts_adjustment_rh_temp_firebase==""){
    service_reset();
  }

  
  delay_server = delay_server_firebase; 
  Serial.println(delay_server);
  sts_adjustment_rh_temp = sts_adjustment_rh_temp_firebase;
  Serial.println(sts_adjustment_rh_temp);
  ////////////////////////////////////////////////////////////////////////
  
  char *sts_adjustment_rh_tempp = strdup(sts_adjustment_rh_temp.c_str());
  char * rh = strtok(sts_adjustment_rh_tempp,"@");
  char * temp = strtok(NULL,"@");

  rh_call=atoi(rh);
  temp_call=atoi(temp);
  
  
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
//  Thread5.onRun(service_timeout);
//  Thread5.setInterval(30000);
  Serial.println("device ready "+version_firmware);
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void service(){
  
  if(WiFi.status() == WL_CONNECTED){

    if(pinger.Ping("otoridashboard.id") == false){
      Serial.println("Error during ping command service.");
      sts_server = "0";
      //lakukan cek ping jika ada lanjut get api jika tidak, simpan data to sd card\
      
    }else{
      Serial.println("connected.");
      String pemilik = httpPOSTRequest_pemilik();
      JSONVar var_pemilik = JSON.parse(pemilik);
      pemilik = var_pemilik["pemilik"];
      Serial.println(pemilik);
      
      if(pemilik != ""){
        
        if(pemilik != "no_id_user" && pemilik != "lock"){
          sts_server = "1";
         
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
          sts_server = "0";
          Serial.println("no_id_user");
        }else if(pemilik=="lock"){
          sts_server = "0";
          Serial.println("device lock");
        }else{
          sts_server = "0"; 
          Serial.println("error lain");
        }
        
      }else{
      Serial.println("data pemilik firebase kosong");
      }  
    }
    yield();
  
  }else{
    Serial.println("wifi not connected");
    sts_server = "0"; 
    service_reset();
  }
  yield();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void acpn_mode(){
  if(digitalRead(btn_acpn)==HIGH){
      WiFiManager wifiManager;
      Serial.println("ACPN MODE");
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
    delay(1000);
    if(pinger.Ping("otoridashboard.id") == false){
      Serial.println("Error during ping command service_control.");
      sts_server = "0"; 
    }else{
//      sts_server = "0"; 
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



      if(delay_server_control !="" && sts_adjustment_rh_temp_control !=""){
          if (delay_server != delay_server_control){
            Serial.println("reset by delay");
            service_reset();
          }else if(sts_adjustment_rh_temp != sts_adjustment_rh_temp_control){
            Serial.println("reset by adjust");
            service_reset();
          }else{
            Serial.println("passs data ada");
          }
      }else{
        Serial.println("passss data kosong");
      }
      
    }  
    yield();

  }else{
    Serial.println("error wifi");
    service_reset();
  }

  yield();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
//void service_timeout(){
//  if(WiFi.status() == WL_CONNECTED){
//    if(pinger.Ping("otoridashboard.com") == false){
//      Serial.println("Error during ping command.");
//      sts_server = "0"; 
//    }
//    else{
//       Serial.println("connected.");
//    }
//  }
//  else{
//     service_reset();
//  }
//}
///////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////
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
///////////////////////////////////////////////////////////////////////////////////////////////////
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
///////////////////////////////////////////////////////////////////////////////////////////////////
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
///////////////////////////////////////////////////////////////////////////////////////////////////
void httpPOSTRequest_post_data(String data){
  HTTPClient http;
  http.begin(post_data);
  http.addHeader("Content-Type", "application/json");
//  String params = "{\n\t\"id\":\""+id_device+"\",\n\t\"data\":\""+data+"\"\n}";

  String params = "{\"id\":\""+id_device+"\",\"data\":\""+String(data)+"\"}";
                  
//  String params = "{\"id\":\""+id_device+"\",\"data\":\""+data+"\"}";
  Serial.println(params);
  int body = http.POST(params);
  String payload = http.getString();
  Serial.print("HTTP Response code post data: ");
  Serial.println(body);
  Serial.print("HTTP Response data post data: ");
  Serial.println(payload);
  http.end();
  
  yield();
 
}
///////////////////////////////////////////////////////////////////////////////////////////////////
void loop() {
  if(Thread1.shouldRun())
    Thread1.run();
  if(Thread2.shouldRun())
    Thread2.run();
  if(Thread3.shouldRun())
    Thread3.run();
  if(Thread4.shouldRun())
    Thread4.run();
//  if(Thread5.shouldRun())
//  Thread5.run();
  int x = 0;
  x = 1 + 2;
}
///////////////////////////////////////////////////////////////////////////////////////////////////
