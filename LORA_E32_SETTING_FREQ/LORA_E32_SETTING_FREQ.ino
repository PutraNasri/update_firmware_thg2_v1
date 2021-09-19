// Transmitter code for Arduino x E32 LoRa

#include <SoftwareSerial.h>
#include "EBYTE.h"
#include <string.h>
#include <SPI.h>
#define PIN_M0 4
#define PIN_M1 5
#define PIN_AX 6
SoftwareSerial ESerial(2, 3);
EBYTE Transceiver(&ESerial, PIN_M0, PIN_M1, PIN_AX);


void setup() {
  Serial.begin(115200);
  while (!Serial);
  
  
  pinMode(PIN_M0, OUTPUT);
  pinMode(PIN_M1, OUTPUT);
  pinMode(PIN_AX, INPUT);
 

  delay(2000);
  Serial.println("Opening LORA bus...");
  ESerial.begin(9600);
  
  
  Transceiver.init();
  Transceiver.SetMode(MODE_PROGRAM);
  Transceiver.SetTransmitPower(OPT_TP20);
  Transceiver.SetUARTBaudRate(UDR_9600);
  Transceiver.SetAirDataRate(ADR_300);
  Transceiver.SetAddressH(0);
  Transceiver.SetAddressL(0);
  Transceiver.SetChannel(20);

  Transceiver.SaveParameters(PERMANENT);
  Transceiver.Reset();

  Transceiver.PrintParameters();
  Transceiver.SetMode(MODE_NORMAL);
  Transceiver.Reset();
  ESerial.flush();
}

void loop() {  
  
  
  

}
