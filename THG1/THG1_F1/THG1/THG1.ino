////////////////////////
////author by Putra////
//////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////
void(* service_reset) (void) = 0;
////////////////////////////////////////////////////////////////////////////////////////////////////
#include <Thread.h>
#include <Pinger.h>
#include <ESP8266WiFi.h>
#include "FirebaseESP8266.h"
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <ESP8266HTTPClient.h>
#include <WiFiManager.h>
#include "DHTesp.h"
//DHTesp dht;
///////////////////////////////////
#include "DHT.h"
#define DHTPIN 5
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);
///////////////////////////////////
#define FIREBASE_HOST "lab-12-05-2020.firebaseio.com"
//#define FIREBASE_HOST "labodia-1affe.firebaseio.com"
//#define FIREBASE_AUTH "x3Uszo07DuQwCHrSRYfWcxiXgfFBUiQdbmOwIusm"  
#define FIREBASE_AUTH "SN7HVRNajTD03IyYX84CosUYUlUO5Qw5kgYKfKRS"
/////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////bagian yang harus di sesuaikan////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////
String id_device      = "ENC54895";
const char* ssid      = "ENC54895";
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
FirebaseData firebaseData1;
FirebaseData firebaseData2;
FirebaseData firebaseData3;
FirebaseData firebaseData4;
FirebaseJson json;

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
  Firebase.begin(FIREBASE_HOST,FIREBASE_AUTH);
  timeClient.begin();
  timeClient.setTimeOffset(25200);
  
  Firebase.getString(firebaseData1,"/IntroAPP/Device/"+id_device+"/delay");
  Firebase.getString(firebaseData2,"/IntroAPP/Device/"+id_device+"/adjustment_rh_temp");

  sts_adjustment_rh_temp = firebaseData2.stringData();
  Serial.println(sts_adjustment_rh_temp);
  
  char *sts_adjustment_rh_tempp = strdup(sts_adjustment_rh_temp.c_str());
  char * rh = strtok(sts_adjustment_rh_tempp,"@");
  char * temp = strtok(NULL,"@");

  rh_call=atoi(rh);
  temp_call=atoi(temp);
  
  delay_server=firebaseData1.stringData();
  Serial.println(delay_server);
  
  if (firebaseData1.stringData() != ""){
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
  Thread5.onRun(service_timeout);
  Thread5.setInterval(30000);
  Serial.println("device ready");
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void service(){
  if(WiFi.status() == WL_CONNECTED){

    if(Firebase.getString(firebaseData3, "/IntroAPP/Device/"+id_device+"/pemilik")){
      
      if(firebaseData3.stringData() != "no_id_user" && firebaseData3.stringData() != "lock"){
        Serial.println(firebaseData3.stringData());
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
        Firebase.pushString(firebaseData4,"/IntroAPP/Device/"+id_device+"/data",data);
        Serial.println(data);
      }
      else if(firebaseData3.stringData() == "no_id_user"){
        sts_server = "0";
        Serial.println("no_id_user");
      }
      else if(firebaseData3.stringData() == "lock"){
        sts_server = "0";
        Serial.println("device lock");
      }
      else{
        sts_server = "0"; 
        Serial.println("error lain");
      }
    }
  }
  else if(WiFi.status() != WL_CONNECTED){
    sts_server = "0"; 
    service_reset();
  }
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
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void service_control(){

  if(WiFi.status() == WL_CONNECTED){
    
    if(Firebase.getString(firebaseData1,"/IntroAPP/Device/"+id_device+"/delay")){
      
      Firebase.getString(firebaseData1,"/IntroAPP/Device/"+id_device+"/delay");
      Firebase.getString(firebaseData2,"/IntroAPP/Device/"+id_device+"/adjustment_rh_temp");

      if (delay_server != firebaseData1.stringData()){
        Serial.println("reset by delay");
        service_reset();
      }
      else if (sts_adjustment_rh_temp != firebaseData2.stringData()){
        Serial.println("reset by adjust");
        service_reset();
      }
    }
    else{
      Serial.println("error ping");
      sts_server = "0"; 
    }
  }
  else{
    Serial.println("error wifi");
    service_reset();
  }
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void service_timeout(){
  if(WiFi.status() == WL_CONNECTED){
    if(pinger.Ping("google.com") == false){
      Serial.println("Error during ping command.");
      sts_server = "0"; 
    }
    else{
       Serial.println("connected.");
    }
  }
  else{
     service_reset();
  }
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
  if(Thread5.shouldRun())
  Thread5.run();
  int x = 0;
  x = 1 + 2;
}
///////////////////////////////////////////////////////////////////////////////////////////////////
