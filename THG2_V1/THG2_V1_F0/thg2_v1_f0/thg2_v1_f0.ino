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


/////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////bagian yang harus di sesuaikan////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////
String id_device      = "PHQ62817";
const char* ssid      = "PHQ62817";
/////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////
const char* password = "12345678";
File myFile;
Pinger pinger;
String version_firmware = "V0";
String Fingerprint = "70 94 DE DD E6 C4 69 48 3A 92 70 A1 48 56 78 2D 18 64 E0 B7";
String get_versionfirmware = "http://otoridashboard.id/versi";
#define URL_update "https://raw.githubusercontent.com/PutraNasri/update_firmware_thg2_v1/main/THG2_V1/V1/ini.bin"
HTTPClient http;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
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
      firmware_version_control = var_firmware_version["delay"];
      Serial.println("delay control = "+firmware_version_control);
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

void loop() {
  // put your main code here, to run repeatedly:
  cek_version_firmware();
  delay(20000);
}
