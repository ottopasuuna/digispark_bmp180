
#define SERIAL_MODE 0

#include "bmp180.h"
#include <avr/eeprom.h>

#if SERIAL_MODE
  #include <SoftSerial.h>
  const int Rx = 3;
  const int Tx = 4;
  SoftSerial sSerial = SoftSerial(Rx, Tx);
 #endif


void setup() {
  // put your setup code here, to run once:
  #if SERIAL_MODE
  sSerial.begin(9600);
  #endif

    // Enable I2C
  Wire.begin();
  

  readCoefficients();

}

void loop() {
  // put your main code here, to run repeatedly:
  #if SERIAL_MODE
  //sSerial.println(readRawTemperature());
  //sSerial.println(eeprom_read_float(0));
  sSerial.println(compensatedTemp());
  #else
  digitalWrite(1, HIGH);
  delay(1000);
  digitalWrite(1, LOW);
  eeprom_write_float(0, compensatedTemp());
  #endif
  delay(1000);


}
