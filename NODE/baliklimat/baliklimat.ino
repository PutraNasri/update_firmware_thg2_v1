
//#include "AESLib.h"
//AESLib aesLib;
#include <Base64.h>
#include <SoftwareSerial.h>
#include "EBYTE.h"
#include <Sleep_n0m1.h>
#include <string.h>
#include <SPI.h>
#define PIN_M0 4
#define PIN_M1 5
#define PIN_AX 6
SoftwareSerial ESerial(2, 3);
EBYTE Transceiver(&ESerial, PIN_M0, PIN_M1, PIN_AX);
int pin_relay = 7 ; 
const int analogInPin_1 = A0;
const int analogInPin_2 = A1;
const int analogInPin_3 = A2;
const int analogInPin_4 = A3;
int sensorValue_1 = 0;
float outputValue_1 = 0; 
int sensorValue_2 = 0;
float outputValue_2 = 0; 
int sensorValue_3 = 0;
float outputValue_3 = 0; 
int sensorValue_4 = 0;
float outputValue_4 = 0; 
float volt_1 = 0;
float volt_2 = 0;
float volt_3 = 0;
float volt_4 = 0;


//char cleartext[256];
//char ciphertext[512];
//// AES Encryption Key
//byte aes_key[] = { 0x15, 0x2B, 0x7E, 0x16, 0x28, 0xAE, 0xD2, 0xA6, 0xAB, 0xF7, 0x15, 0x88, 0x09, 0xCF, 0x4F, 0x3C };
//// General initialization vector (you must use your own IV's in production for full security!!!)
//byte aes_iv[N_BLOCK] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
//// Generate IV (once)
 

//void aes_init() {
//  aesLib.gen_iv(aes_iv);
//  // workaround for incorrect B64 functionality on first run...
//  encrypt("HELLO WORLD!", aes_iv);
//}
// 
//String encrypt(char * msg, byte iv[]) {  
//  int msgLen = strlen(msg);
//  char encrypted[2 * msgLen];
//  aesLib.encrypt64(msg, msgLen, encrypted, aes_key,sizeof(aes_key) ,iv);  
//  return String(encrypted);
//}
//
//String decrypt(char * msg, byte iv[]) {
//  unsigned long ms = micros();
//  int msgLen = strlen(msg);
//  char decrypted[msgLen]; // half may be enough
//  aesLib.decrypt64(msg, msgLen, decrypted, aes_key,sizeof(aes_key) ,iv);  
//  return String(decrypted);
//}

void setup() {
  Serial.begin(115200);
  
  while (!Serial);
//  aes_init();

  pinMode(PIN_M0, OUTPUT);
  pinMode(PIN_M1, OUTPUT);
  pinMode(PIN_AX, INPUT);
  pinMode(pin_relay, OUTPUT);
  pinMode(analogInPin_1, INPUT);
  pinMode(analogInPin_2, INPUT);
  pinMode(analogInPin_3, INPUT);
  pinMode(analogInPin_4, INPUT);

  Serial.println("Opening LORA bus...");
  ESerial.begin(9600);

  
}
 
void loop() {
  //  nyalakan relay
  digitalWrite(pin_relay,HIGH);
  delay(3000);

  sensor_1();
  Serial.print("sensor_1 = ");
  Serial.println(sensorValue_1);
  Serial.print("volt_1 = ");
  Serial.println(String(volt_1));
  delay(1000);

  sensor_2();
  Serial.print("sensor_2 = ");
  Serial.println(sensorValue_2);
  Serial.print("volt_2 = ");
  Serial.println(String(volt_2));
  delay(1000);

  sensor_3();
  Serial.print("sensor_3 = ");
  Serial.println(sensorValue_3);
  Serial.print("volt_3 = ");
  Serial.println(String(volt_3));
  delay(1000);

  sensor_4();
  Serial.print("sensor_4 = ");
  Serial.println(sensorValue_4);
  Serial.print("volt_4 = ");
  Serial.println(String(volt_4));
  delay(1000);


  
  //matikan relay
  digitalWrite(pin_relay,LOW);
  delay(3000);

  String str = "node123@"+String(volt_1)+"@"+String(volt_2)+"@"+String(volt_3)+"@"+String(volt_4)+"@nan@nan";
  char Buf[50];
//  String str = "node123@1@2@3@4@nan@nan";
//  str.toCharArray(cleartext, str.length()+1);
//  
//  byte enc_iv[N_BLOCK] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }; // iv_block gets written to, provide own fresh copy...
//  String encrypted = encrypt(cleartext, enc_iv);
//  sprintf(ciphertext, "%s", encrypted.c_str());
//  Serial.print("Ciphertext encrypted: ");
//  Serial.println(encrypted);
//
//  encrypted.toCharArray(ciphertext, encrypted.length()+1);
//  byte dec_iv[N_BLOCK] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }; // iv_block gets written to, provide own fresh copy...
//  String decrypted, buf_decrypted;
//  decrypted = decrypt(ciphertext, dec_iv);  
//  Serial.print("data Decrypted: ");
//  Serial.println(decrypted);

  // encoding
  char input[] = "node123@1@2@3@4@nan@nan";
  //strcat(input,'\0');
  int inputLen = sizeof(input);
  
  int encodedLen = base64_enc_len(inputLen);
  char encoded[encodedLen];
  
  Serial.print(input); Serial.print(" = ");
  
  // note input is consumed in this step: it will be empty afterwards
  base64_encode(encoded, input, inputLen); 
  
  Serial.println(encoded);


  String data_encoded = encoded;

//  // decoding
//  char input2[] = "bm9kZTEyM0AxQDJAM0A0QG5hbkBuYW4A";
//  int input2Len = sizeof(input2);
//  
//  int decodedLen = base64_dec_len(input2, input2Len);
//  char decoded[decodedLen];
//  
//  base64_decode(decoded, input2, input2Len);
//  
//  Serial.print(input2); Serial.print(" = "); Serial.print(decoded);
  

  ESerial.println(data_encoded);
  
  delay(25000);
}

void sensor_1(){
  sensorValue_1 = analogRead(analogInPin_1);
  volt_1 = (sensorValue_1 * 5.0)/1024;
}

void sensor_2(){
  sensorValue_2 = analogRead(analogInPin_2);
  volt_2 = (sensorValue_2 * 5.0)/1024;
}

void sensor_3(){
  sensorValue_3 = analogRead(analogInPin_3);
  volt_3 = (sensorValue_3 * 5.0)/1024;
}

void sensor_4(){
  sensorValue_4 = analogRead(analogInPin_4);
  volt_4 = (sensorValue_4 * 5.0)/1024;
}
 
