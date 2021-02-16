#include <SPI.h>
#include <LoRa.h>
#include <Sleep_n0m1.h>
#include <SDISerial.h>
#include <string.h>
#define DATA_ATMOS14 5
SDISerial connection_ATMOS14(DATA_ATMOS14);
char output_buffer[125]; // just for uart prints
char tmp_buffer[4];


Sleep sleep;
unsigned long sleepTime = 900000;



String id_device = "xxxxxxxx";



void setup() {
  connection_ATMOS14.begin();
  Serial.begin(9600);
  char* resp = connection_ATMOS14.sdi_query("1CC!",1000);
  delay(3000);//3 seconds should be more than enough

  while (!Serial);

  Serial.println("LoRa Sender");

  if (!LoRa.begin(434E6)) {
    Serial.println("Starting LoRa failed!");
    while (1);
  }
  Serial.println("LoRa init succeeded.");
  Serial.println();
  Serial.println("LoRa Simple Node");
  Serial.println("Only receive messages from gateways");
}

void loop() {
  char* resp = connection_ATMOS14.sdi_query("1D0!",1000);
  sprintf(output_buffer,"%s",resp?resp:"No Response Recieved!!");
  char *a = strtok(output_buffer,"+");
  char *b = strtok(NULL,"+");
  char *c = strtok(NULL,"+");
  char *d = strtok(NULL,"+");
  char *e = strtok(NULL,"+");
  e[strlen(e)-4] ='\0';
  String Vapor_pressure_in_kPa = String(b);
  String Temperature = String(c);
  String Relative_humidity= String(d);
  String Atmospheric_Pressure_in_kPa = String(e);
  
  Serial.println("Vapor pressure in kPa = "+String(b));
  Serial.println("Temperature = "+String(c));
  Serial.println("Relative humidity = "+String(d));
  Serial.println("Atmospheric Pressure in kPa = "+String(e));
  Serial.println("");

  
//  sensorValue = analogRead(analogInPin);
//  outputValue = map(sensorValue, 0, 1023, 0, 255);
//  float milivolt = sensorValue * 5.0;
//  Serial.println(milivolt);
  
  String data_no_encryp = id_device+"@"+Vapor_pressure_in_kPa+"@"+Temperature+"@"+Relative_humidity+"@"+Atmospheric_Pressure_in_kPa+"@nan@nan";
  
  
//  Serial.print("Ciphertext: ");
//  Serial.println(data_encryp);
  
  
  
  
  delay(100);
  
  // send packet
  Serial.println("Sending packet: ");
  delay(100);
  LoRa.beginPacket();
  LoRa.print(data_no_encryp);
  LoRa.endPacket();
 // sleep nodes
  Serial.println("Start Sleep");
  digitalWrite(LED_BUILTIN,LOW);
  delay(100);
  sleep.pwrDownMode();
  sleep.sleepDelay(sleepTime);
  Serial.println("Bangun");
  
}
