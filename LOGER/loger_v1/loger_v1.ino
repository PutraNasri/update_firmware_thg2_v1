// TTGO T-Call pin definitions
#define MODEM_RST            5
#define MODEM_PWKEY          4
#define MODEM_POWER_ON       23
#define MODEM_TX             27
#define MODEM_RX             26
#define I2C_SDA              21
#define I2C_SCL              22

#define TINY_GSM_MODEM_SIM800

#include <TinyGsmClient.h>

#include <Wire.h>
// #include <TinyGsmClient.h>
#include "utilities.h"

// Set serial for debug console (to the Serial Monitor, default speed 115200)
#define SerialMon Serial

// Hardware Serial on Mega, Leonardo, Micro
#define SerialAT Serial1


const char apn[]  = "telkomsel";
const char user[] = "WAP";
const char pass[] = "WAP123";

TinyGsm modem(SerialAT);


#include <Arduino.h>
#include <WiFiManager.h>
#include <NTPClient.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <Arduino_JSON.h>
#include <WiFiUdp.h>
#include <DNSServer.h>
#include <Wire.h>
#include "RTClib.h"


#include "FS.h"
#include "SD.h"
#include "SPI.h"


const char* remote_host = "https://otori-279015.df.r.appspot.com";
String get_pemilik = "https://otori-279015.df.r.appspot.com/log/pemilik";
String get_adjustment = "https://otori-279015.df.r.appspot.com/log/adjustment";
String get_delay = "https://otori-279015.df.r.appspot.com/log/delay";
String get_versionfirmware = "https://otori-279015.df.r.appspot.com/log/versionfirmware";
String post_data = "https://otori-279015.df.r.appspot.com/log";
String Fingerprint = "null";
#define URL_update "https://raw.githubusercontent.com/PutraNasri/update_firmware_thg2_v1/main/THG2_V1/ini.bin"

String id_device      = "xxxxxxxloger";
const char* ssid      = "rumah singgah";
const char* password = "nanonano";
String sts_server = "0";
int menit_to_detik = 60;
int detik_to_milidetik = 1000;
String delay_server = "";
String delay_offline = "";
String pemilik = "";
String connection = "";
int pin_relay = 12;
int pin_led_1 = 32;
int pin_led_2 = 33;
//int pin_led_3 = 27;

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP);
File myFile;
String formattedDate;
String dayStamp;
String timeStamp;



#if defined(ARDUINO_ARCH_SAMD)
   #define Serial SerialUSB
#endif
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





void setup() {
  SerialMon.begin(115200);
  delay(10);
  
  pinMode(pin_relay,OUTPUT);
  pinMode(pin_led_1,OUTPUT);
  pinMode(pin_led_2,OUTPUT);
//  pinMode(pin_led_3,OUTPUT);
  delay(1000);
  digitalWrite(pin_relay,HIGH);
  digitalWrite(pin_led_1,HIGH);
  digitalWrite(pin_led_2,HIGH);
//  digitalWrite(pin_led_3,HIGH);
////////////////////////////////////////////////////////////////////////////////
  Wire.begin(I2C_SDA, I2C_SCL);
  bool   isOk = setPowerBoostKeepOn(1);
  Serial.println(String("IP5306 KeepOn ") + (isOk ? "OK" : "FAIL"));

  // Set-up modem reset, enable, power pins
  pinMode(MODEM_PWKEY, OUTPUT);
  pinMode(MODEM_RST, OUTPUT);
  pinMode(MODEM_POWER_ON, OUTPUT);

  digitalWrite(MODEM_PWKEY, LOW);
  digitalWrite(MODEM_RST, HIGH);
  digitalWrite(MODEM_POWER_ON, HIGH);

  // Set GSM module baud rate and UART pins
  SerialAT.begin(115200, SERIAL_8N1, MODEM_RX, MODEM_TX);
  delay(3000);

  // Restart takes quite some time
  // To skip it, call init() instead of restart()
  SerialMon.println("Initializing modem...");
  modem.restart();

  String modemInfo = modem.getModemInfo();
  SerialMon.print("Modem: ");
  SerialMon.println(modemInfo);

  // Unlock your SIM card with a PIN
  //modem.simUnlock("1234");

   SerialMon.print("Waiting for network...");
  if (!modem.waitForNetwork(240000L)) {
    SerialMon.println(" fail");
    delay(5000);
    return;
  }
  SerialMon.println(" OK");

  if (modem.isNetworkConnected()) {
    SerialMon.println("Network connected");
  }

  SerialMon.print(F("Connecting to APN: "));
  SerialMon.print(apn);
  if (!modem.gprsConnect(apn, user, pass)) {
    SerialMon.println(" fail");
    delay(10000);
    return;
  }
  SerialMon.println(" OK");

  //////////////////////////////////////////////////////////////////////////////////


//  Serial.println("Connecting to ");
//  WiFi.mode(WIFI_STA); // Setup ESP in client mode
//  WiFi.begin(ssid, password); // Connect to wifi

//  WiFiManager wifiManager;
//  wifiManager.setTimeout(60);
//  wifiManager.setBreakAfterConfig(true);
//  if (!wifiManager.autoConnect(ssid, password)) {
//    Serial.println("failed to connect, we should reset as see if it connects");
////    continue;
//  }
  delay(2000);
  Serial.println("");
  Serial.println("WiFi connected.");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  
  delay(2000);
//////////////////////////////////////////////////////////////////////////////////
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
  Serial.println("disini");
  
  if (!SD.begin(5)) {
    Serial.println("Gagal Memuat Kartu SD");
    return;
  }
  uint8_t cardType = SD.cardType();
  if (cardType == CARD_NONE) {
    Serial.println("Tidak Ada Kartu SD");
    return;
  }
//  87651234
  Serial.println("initialization SD done.");
//  deleteFile(SD, "/id_device.txt"); 
  delay(1000);
  listDir(SD, "/", 0);
  delay(1000);
//  appendFile(SD, "/config_offline.txt", "2\r\n");
//  delay(1000);
//  writeFile(SD, "/id_device.txt", "TESTINGDATA\r\n");
  delay(1000);
  readFile_id(SD, "/id_device.txt");
  delay(1000);
  readFile_config(SD, "/config.txt");
  delay(1000);
  readFile_connection(SD, "/connection.txt");
  delay(1000);
  readFile_config_offline(SD, "/config_offline.txt");
  delay(1000);
//  writeFile(SD, "/id_device.txt", "TESTINGDATA");


  timeClient.begin();
  timeClient.setTimeOffset(25200);
  



  if (connection == "0"){
    Serial.println("id device = "+id_device);
    Serial.println("mode offline = "+connection);
    Serial.println("delay offline = "+delay_offline);
    // Define deep sleep options
    uint64_t uS_TO_S_FACTOR = 1000000;  // Conversion factor for micro seconds to seconds
    // Sleep for 2 minutes = 120 seconds
    uint64_t TIME_TO_SLEEP = delay_offline.toInt()*menit_to_detik;
    
    esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP * uS_TO_S_FACTOR);
    
    xTaskCreate(
          log_data_offline,
          "Task log data offline",
          5000,
          NULL,
          1,
          NULL);
  }
  else if(connection == "1"){
    Serial.println("mode online = "+connection);
    xTaskCreate(
          Task1,
          "Task 1",
          5000,
          NULL,
          1,
          NULL);
          
    xTaskCreate(
        Task2,
        "Task 2",
        5000,
        NULL,
        1,
        NULL);
    
  }else{
    
  }
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

void writeFile(fs::FS &fs, const char * path, const char * message){
    Serial.printf("Writing file: %s\n", path);

    File file = fs.open(path, FILE_WRITE);
    if(!file){
        Serial.println("Failed to open file for writing");
        return;
    }
    if(file.print(message)){
        Serial.println("File written");
    } else {
        Serial.println("Write failed");
    }
    file.close();
}

void readFile_id(fs::FS &fs, const char * path){
//    Serial.printf("Reading file: %s\n", path);
    String id_device_sdcard;
    File file = fs.open(path);
    if(!file){
        Serial.println("Failed to open file for reading");
        return;
    }

//    Serial.print("Read from file: ");
    while(file.available()){
//        Serial.write(file.read());
        id_device_sdcard = String(file.readStringUntil('\n'));
        id_device_sdcard.trim(); 
        id_device = id_device_sdcard;
        
    }
    file.close();
//    Serial.println("ini id device = " +id_device);
}

void readFile_config(fs::FS &fs, const char * path){
//    Serial.printf("Reading file: %s\n", path);
    File file = fs.open(path);
    if(!file){
        Serial.println("Failed to open file for reading");
        return;
    }

//    Serial.print("Read from file: ");
    while(file.available()){
        Serial.write(file.read()); 
        
    }
    file.close();
}

void readFile_config_offline(fs::FS &fs, const char * path){
//    Serial.printf("Reading file: %s\n", path);
    String data_config_offline_sd_card;
    File file = fs.open(path);
    if(!file){
        Serial.println("Failed to open file for reading");
        return;
    }

//    Serial.print("Read from file: ");
    while(file.available()){
//        Serial.write(file.read()); 
        data_config_offline_sd_card = String(file.readStringUntil('\n'));
        data_config_offline_sd_card.trim();
        delay_offline = data_config_offline_sd_card;
    }
    file.close();
    
}

void readFile_connection(fs::FS &fs, const char * path){
//    Serial.printf("Reading file: %s\n", path);
    String data_connection_in_sdcard;
    File file = fs.open(path);
    if(!file){
        Serial.println("Failed to open file for reading");
        return;
    }

//    Serial.print("Read from file: ");
    while(file.available()){
//        Serial.write(file.read());
        data_connection_in_sdcard = String(file.readStringUntil('\n'));
        data_connection_in_sdcard.trim(); 
        connection = data_connection_in_sdcard;
        
        
    }
    file.close();
//    Serial.println("ini data connection  = " +connection);
}

void listDir(fs::FS &fs, const char * dirname, uint8_t levels){
    Serial.printf("Listing directory: %s\n", dirname);

    File root = fs.open(dirname);
    if(!root){
        Serial.println("Failed to open directory");
        return;
    }
    if(!root.isDirectory()){
        Serial.println("Not a directory");
        return;
    }

    File file = root.openNextFile();
    while(file){
        if(file.isDirectory()){
            Serial.print("  DIR : ");
            Serial.println(file.name());
            if(levels){
                listDir(fs, file.name(), levels -1);
            }
        } else {
            Serial.print("  FILE: ");
            Serial.print(file.name());
            Serial.print("  SIZE: ");
            Serial.println(file.size());
        }
        file = root.openNextFile();
    }
}

void appendFile(fs::FS &fs, const char * path, const char * message) {
  Serial.printf("Appending to file: %s\n", path);

  File file = fs.open(path, FILE_APPEND);
  if(!file) {
    Serial.println("Failed to open file for appending");
    return;
  }
  if(file.print(message)) {
    Serial.println("Message appended");
  } else {
    Serial.println("Append failed");
  }
  file.close();
}

void deleteFile(fs::FS &fs, const char * path){
    Serial.printf("Deleting file: %s\n", path);
    if(fs.remove(path)){
        Serial.println("File deleted");
    } else {
        Serial.println("Delete failed");
    }
}

void Task1(void *parameter){

  for( ;; ){
    delay(2000);
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
      WiFi.begin(ssid, password);
    }
    
  vTaskDelay(50000 / portTICK_PERIOD_MS);
  }
}

void log_data_offline(void *parameter){

  for( ;; ){
    ////////////
    
    //aktifkan relay power sensor
    digitalWrite(pin_relay,LOW);
    Serial.println("relay on");
    
//    sinkronisasi_waktu();
    
    delay(3000);
    
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
    String data = String(date_rtc)+"@"+String(time_rtc);
    Serial.println(data);
    //get data sensor
    //get data time
    //save data to sd card
    //non aktifkan relay power sensor
    ////////////
    vTaskDelay(delay_offline.toInt()*menit_to_detik*detik_to_milidetik / portTICK_PERIOD_MS); 
    
//    esp_sleep_pd_config(ESP_PD_DOMAIN_MAX, ESP_PD_OPTION_OFF);
//    esp_sleep_pd_config(ESP_PD_DOMAIN_RTC_PERIPH, ESP_PD_OPTION_OFF);
//    esp_sleep_pd_config(ESP_PD_DOMAIN_RTC_SLOW_MEM, ESP_PD_OPTION_OFF);
//    esp_sleep_pd_config(ESP_PD_DOMAIN_RTC_FAST_MEM, ESP_PD_OPTION_OFF);
//    esp_deep_sleep_start();
//    
//    esp_sleep_pd_config(ESP_PD_DOMAIN_RTC_PERIPH,ESP_PD_OPTION_OFF);
  }
}



void loop() {
  
}
