////////////////////////
////author by Putra////
/////////THG2_V2//////////
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
#include <OneWire.h>
//#include <SPI.h>
#include <SD.h>
//DHTesp dht;
///////////////////////////////////
int DS18S20_Pin = 0; //DS18S20 Signal pin on digital 2
OneWire ds(DS18S20_Pin);
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
String get_pemilik = "http://otoridashboard.id/get_pemilik";
String get_adjustment_rh_temp = "http://otoridashboard.id/get_adjustment_rh_temp";
String get_delay = "http://otoridashboard.id/get_delay";
String post_data = "http://otoridashboard.id/nulis_data";
/////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////bagian yang harus di sesuaikan////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////
String id_device      = "PHQ62817";
const char* ssid      = "PHQ62817";
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
Thread Thread5 = Thread();

////////////////////////////////////////////////////////////////////////////////////////////////////
void setup() {
 
  
  Serial.begin(115200);
  
  pinMode(btn_acpn,INPUT);
//  pinMode(pin_led_acpn,OUTPUT);
//  pinMode(pin_led_no_ok,OUTPUT);
//  pinMode(pin_led_ok,OUTPUT);
  
  delay(1000);
  u8g2.begin();
 
  Serial.print("Connecting to ");
  WiFiManager wifiManager;
  wifiManager.setTimeout(60);
  wifiManager.setBreakAfterConfig(true);
//  digitalWrite(pin_led_no_ok,LOW);
//  digitalWrite(pin_led_ok,LOW);
//  digitalWrite(pin_led_acpn,HIGH);
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
//  digitalWrite(pin_led_no_ok,HIGH);
//  digitalWrite(pin_led_ok,LOW);
//  digitalWrite(pin_led_acpn,LOW);
  
  timeClient.begin();
  timeClient.setTimeOffset(25200);
  
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }
  
  Serial.print("Initializing SD card...");

  if (!SD.begin(15)) {
    Serial.println("initialization failed!");
    while (1);
  }
  Serial.println("initialization done.");
//////////////////////////////////////////////////////////////////////////////////////////
  delay(1000);
  String delay_server_firebase = httpPOSTRequest_delay();
  JSONVar var_delay_firebase = JSON.parse(delay_server_firebase);
  delay_server_firebase = var_delay_firebase["delay"];
  Serial.println("delay firebase = "+delay_server_firebase);
  
  delay(500);
  String sts_adjustment_rh_temp_firebase = httpPOSTRequest_adjustment_rh_temp();
  JSONVar var_adjustment_rh_temp_firebase =JSON.parse(sts_adjustment_rh_temp_firebase);
  sts_adjustment_rh_temp_firebase = var_adjustment_rh_temp_firebase["adjustment_rh_temp"];
  Serial.println("adjustment firebase = "+sts_adjustment_rh_temp_firebase);

  delay_server = delay_server_firebase; 
  Serial.println(delay_server);
  sts_adjustment_rh_temp = sts_adjustment_rh_temp_firebase;
  Serial.println(sts_adjustment_rh_temp);
  
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
  Thread5.onRun(service_lcd);
  Thread5.setInterval(2000);
  Serial.println("device ready");

 
}
////////////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////////////////

void service(){
  if(WiFi.status() == WL_CONNECTED){
    
    if(pinger.Ping("otoridashboard.id") == false){
      Serial.println("Error during ping command service.");
      sts_server = "0";
      tulis_sd_card(); 
      //lakukan cek ping jika ada lanjut get api jika tidak, simpan data to sd card\
      
    }
    else{
      //
      Serial.println("connected.");
      //cek isi sd card, jika ada lakukan post data satu persatu,jika sd card kosong lanjut eksekusi api


      cek_data_sdcard_and_send_to_firebase();


      //////////////////////////////////////////////////////////////////////////////////////////////////
      String pemilik = httpPOSTRequest_pemilik();
      JSONVar var_pemilik = JSON.parse(pemilik);
      pemilik = var_pemilik["pemilik"];
      Serial.println(pemilik);
      
      if(pemilik != "no_id_user" && pemilik != "lock"){
        sts_server = "1";
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
  //      Serial.println(data);
        
        httpPOSTRequest_post_data(data);
        
//          delay(500);
//          tulis_sd_card(); 
        
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
    }
    
    yield();
  } 
  
  
  ///////////////////////wifi can't connect//////////////////////////////////////////
  else if(WiFi.status() != WL_CONNECTED){
    
    sts_server = "0"; 
    
    tulis_sd_card(); //tulis di sdcard
    
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
      Serial.println("ACPN MODE");
//      digitalWrite(pin_led_no_ok,LOW);
//      digitalWrite(pin_led_ok,LOW);
//      digitalWrite(pin_led_acpn,HIGH);
      wifiManager.resetSettings();
      wifiManager.autoConnect(ssid, password);
      wifiManager.setBreakAfterConfig(true);   
  }
  yield();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void led_conf(){
//  if(sts_server == "0"){
//    digitalWrite(pin_led_no_ok,HIGH);
//    digitalWrite(pin_led_ok,LOW);
//    digitalWrite(pin_led_acpn,LOW);
//  }
//  else if (sts_server == "1"){
//    digitalWrite(pin_led_no_ok,LOW);
//    digitalWrite(pin_led_ok,HIGH);
//    digitalWrite(pin_led_acpn,LOW);
//  }
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
    }
    else{
      sts_server = "0"; 
      Serial.println("cek service control");
      String delay_server_control = httpPOSTRequest_delay();
      JSONVar var_delay_control = JSON.parse(delay_server_control);
      delay_server_control = var_delay_control["delay"];
      Serial.println("delay control = "+delay_server);
      delay(1000);
      
      String sts_adjustment_rh_temp_control = httpPOSTRequest_adjustment_rh_temp();
      JSONVar var_adjustment_rh_temp_control =JSON.parse(sts_adjustment_rh_temp_control);
      sts_adjustment_rh_temp_control = var_adjustment_rh_temp_control["adjustment_rh_temp"];
      Serial.println("adjustment control = "+sts_adjustment_rh_temp_control);
  
      if (delay_server != delay_server_control){
      
        Serial.println("reset by delay");
        service_reset();
      }
      else if(sts_adjustment_rh_temp != sts_adjustment_rh_temp_control){
        Serial.println("reset by adjust");
        service_reset();
      }
      yield();
    }

    yield();

    
    
  }else if (WiFi.status() != WL_CONNECTED){
    Serial.println("error wifi");
  }

  yield();
  
}


////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////
void service_lcd(){
 
  float temperature = getTemp();
  float hum = 00;
  Serial.println("temp = "+String(temperature)+"  ///  humd = "+String(hum));
//  
  u8g2.clearBuffer();          // clear the internal memory
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_logisoso42_tr); // choose a suitable font 42 pixel
  u8g2.setCursor(2,42);
  u8g2.print(String(int(temperature+temp_call))+" C");
  u8g2.setCursor(2,95);
  u8g2.print(String(int(hum+rh_call)));  
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
  Serial.println("ini data yang di terima di post = "+data);
  Serial.println("ini id yang di terima di post = "+id_device);
  HTTPClient http;
  http.begin(post_data);
  http.addHeader("Content-Type", "application/json");
  String params = "{\"id\":\""+id_device+"\",\"data\":\""+String(data)+"\"}";
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
///////////////////////////////////////////////////////////////
void tulis_sd_card(){
  // open the file. note that only one file can be open at a time,
  // so you have to close this one before opening another.
  Serial.println("menulis sdcard");
  myFile = SD.open("test.txt", FILE_WRITE);

  // if the file opened okay, write to it:
  if (myFile) {
    Serial.print("Writing to test.txt...");
    ///////////////////////////////////////////////////////////////////////////
    float h = 00;
    float temperature = getTemp();
    while(!timeClient.update()) {
          timeClient.forceUpdate();
    }
    formattedDate = timeClient.getFormattedDate();
    int splitT = formattedDate.indexOf("T");
    dayStamp = formattedDate.substring(0, splitT);
    timeStamp = formattedDate.substring(splitT+1, formattedDate.length()-1);
    String data = String(temperature+temp_call)+"@"+String(h+rh_call)+"@"+String(dayStamp)+"@"+String(timeStamp);
    //////////////////////////////////////////////////////////////////////////
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
        data = myFile.readStringUntil('\n');
        data.trim();
        httpPOSTRequest_post_data(data);
        Serial.println("ini datasd = "+data);
        delay(3000);      
      }
      myFile.close(); 
      SD.remove("test.txt");    
    }else{
      Serial.println("error opening test.txt");
    }   
  }else{
    Serial.println("data sd kosong");
    //lanjut pass
  }
 
}
////////////////////////////////////////////////////////////////////////////////////////////////
float getTemp(){
  //returns the temperature from one DS18S20 in DEG Celsius

  byte data[12];
  byte addr[8];

  if ( !ds.search(addr)) {
      //no more sensors on chain, reset search
      ds.reset_search();
      return -1000;
  }

  if ( OneWire::crc8( addr, 7) != addr[7]) {
      Serial.println("CRC is not valid!");
      return -1000;
  }

  if ( addr[0] != 0x10 && addr[0] != 0x28) {
      Serial.print("Device is not recognized");
      return -1000;
  }

  ds.reset();
  ds.select(addr);
  ds.write(0x44,1); // start conversion, with parasite power on at the end

  byte present = ds.reset();
  ds.select(addr);
  ds.write(0xBE); // Read Scratchpad


  for (int i = 0; i < 9; i++) { // we need 9 bytes
    data[i] = ds.read();
  }

  ds.reset_search();

  byte MSB = data[1];
  byte LSB = data[0];

  float tempRead = ((MSB << 8) | LSB); //using two's compliment
  float TemperatureSum = tempRead / 16;

  return TemperatureSum;

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
