
#include <SPI.h>
#include <LoRa.h>
#include <Sleep_n0m1.h>
#include "AESLib.h"

AESLib aesLib;
Sleep sleep;
unsigned long sleepTime = 900000;
char cleartext[256];
char ciphertext[512];
// AES Encryption Key
byte aes_key[] = { 0x15, 0x2B, 0x7E, 0x16, 0x28, 0xAE, 0xD2, 0xA6, 0xAB, 0xF7, 0x15, 0x88, 0x09, 0xCF, 0x4F, 0x3C };
// General initialization vector (you must use your own IV's in production for full security!!!)
byte aes_iv[N_BLOCK] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
// Generate IV (once)

const int analogInPin = A1;
int sensorValue = 0;
int outputValue = 0; 
String id_device = "xxxxxxxx";



void setup() {
  Serial.begin(9600);
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

  sensorValue = analogRead(analogInPin);
  outputValue = map(sensorValue, 0, 1023, 0, 255);
  float milivolt = sensorValue * 5.0;
  String data_no_encryp = id_device+"@"+milivolt+"@nan@nan@nan@nan@nan";
  data_no_encryp.toCharArray(cleartext,data_no_encryp.length()+1);
  byte enc_iv[N_BLOCK] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }; // iv_block gets written to, provide own fresh copy...
  String data_encryp = encrypt(cleartext, enc_iv);
  sprintf(ciphertext, "%s", data_encryp.c_str());
//  Serial.print("Ciphertext: ");
//  Serial.println(data_encryp);
  
  
  Serial.println(milivolt);
  
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

String encrypt(char * msg, byte iv[]) {  
  int msgLen = strlen(msg);
  char encrypted[2 * msgLen];
  aesLib.encrypt64(msg, msgLen, encrypted, aes_key,sizeof(aes_key) ,iv);  
  return String(encrypted);
}
