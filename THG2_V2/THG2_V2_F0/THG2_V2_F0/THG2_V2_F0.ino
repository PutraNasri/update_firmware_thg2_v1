////////////////////////
////author by Putra////
/////////THG2//////////
////////////////////////////////////////////////////////////////////////////////////////////////////
void(* service_reset) (void) = 0;
////////////////////////////////////////////////////////////////////////////////////////////////////
#include <Pinger.h>
#include <ESP8266WiFi.h>
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <ESP8266HTTPClient.h>
#include <ESP8266httpUpdate.h>
#include <WiFiClient.h>
#include <Arduino_JSON.h>
#include <WiFiManager.h>
#include <SD.h>
#include <SPI.h>
#include <OneWire.h>
int DS18S20_Pin = 0; //DS18S20 Signal pin on digital 2
OneWire ds(DS18S20_Pin);

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
/////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////bagian yang harus di sesuaikan////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////
String id_device      = "YRZ57475";
const char* ssid      = "YRZ57475";
/////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////
const char* password = "12345678";
const int btn_acpn = 16;
const int pin_led_acpn = 1;    
const int pin_led_no_ok = 2;   
const int pin_led_ok = 3;  
File myFile;
Pinger pinger;
String version_firmware = "F0";
String Fingerprint = "70 94 DE DD E6 C4 69 48 3A 92 70 A1 48 56 78 2D 18 64 E0 B7";
String get_versionfirmware = "http://otoridashboard.id/thg2/versi";
#define URL_update "https://raw.githubusercontent.com/PutraNasri/update_firmware_thg2_v1/main/THG2_V2/ini.bin"
HTTPClient http;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  pinMode(btn_acpn,INPUT);
//  pinMode(pin_sensor,INPUT);
  
//  pinMode(pin_led_acpn,OUTPUT);
//  pinMode(pin_led_no_ok,OUTPUT);
//  pinMode(pin_led_ok,OUTPUT);
//  
  u8g2.begin();
//  dht.begin();
  u8g2.clearBuffer();
  u8g2.clearBuffer();
  u8g2.clearBuffer();
  
//  digitalWrite(pin_led_no_ok,HIGH); //HIJAU
//  digitalWrite(pin_led_ok,HIGH);   //KUNING
//  digitalWrite(pin_led_acpn,HIGH); //MERAH
  delay(1000);
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
  //////////////////////////////////////////////////////////////////////////
  if(SD.exists("id_device.txt")){
    Serial.println("file sudah ada");
    myFile = SD.open("id_device.txt");
    String id_device_sdcard;
    int count=1;
    if(myFile){
      Serial.println("id_device.txt:");
      while(myFile.available()){
        id_device_sdcard = String(myFile.readStringUntil('\n'));
        id_device_sdcard.trim();
        Serial.println("ini id device di sd card = "+id_device_sdcard);
        delay(3000);
      }
      myFile.close();
    }else{
      Serial.println("error opening test.txt");
    }
    
  }else{
    Serial.println("menulis id_device to sdcard");
    myFile = SD.open("id_device.txt", FILE_WRITE);
  
    if(myFile){
      Serial.print("Writing to id_device.txt...");
      myFile.println(id_device);
      myFile.close();
      Serial.println("done.");
     
    }else{
      Serial.println("error opening test.txt");
    }
    
  }

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
  
  
}

void cek_version_firmware(){
  if(WiFi.status() == WL_CONNECTED){
    if(pinger.Ping("otoridashboard.id") == false){
      Serial.println("Error during ping command service_control.");
    }else{
      String firmware_version_control = get_version_firmware();
      JSONVar var_firmware_version = JSON.parse(firmware_version_control);
      firmware_version_control = var_firmware_version["versi_firmware"];
      Serial.println("versi firmware control = "+firmware_version_control);
      delay(1000);
      
      if (firmware_version_control != ""){
        if (firmware_version_control != version_firmware){
          Serial.println("New firmware detected.....");
          update_firmware();
        }
        else{
          Serial.println("pass");
        }
      }
      else{  
        Serial.println("pass");
      }
      
    }
    
  }else{
    Serial.println("wifi not connected");
  }
  
  
}

void update_firmware(){
  //////////////////////////
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
        break;

    case HTTP_UPDATE_NO_UPDATES:
      Serial.println("HTTP_UPDATE_NO_UPDATES");
      break;

    case HTTP_UPDATE_OK:
      Serial.println("HTTP_UPDATE_OK");
      break;
  }
}

void service_lcd(){
  String tempp = String(getTemp()+00);
  char * temp = strdup(tempp.c_str());
  Serial.println(tempp);
  
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
  // put your main code here, to run repeatedly:
  cek_version_firmware();
  service_lcd();
  delay(5000);
//  delay(20000);
}
