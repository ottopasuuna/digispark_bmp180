
#define SERIAL_MODE 0
// SERIAL_MODE 0 : read sensor and log to eeprom
// SERIAL_MODE 1 : read eeprom and print to SoftSerial
// SERIAL_MODE 2 : same as 1, but for regular arduino

#include <avr/eeprom.h>

#if SERIAL_MODE == 1
  #include <SoftSerial.h>
  const int Rx = 3;
  const int Tx = 4;
  SoftSerial sSerial = SoftSerial(Rx, Tx);
#elif SERIAL_MODE == 0
  #include <Adafruit_Sensor.h>
  #include "bmp180.h"
 #endif


void setup() {
  // put your setup code here, to run once:
  #if SERIAL_MODE == 1
  sSerial.begin(9600);
  #elif SERIAL_MODE == 2
  Serial.begin(9600);
  #elif SERIAL_MODE == 0
  bmpbegin(BMP085_MODE_ULTRAHIGHRES);
  #endif

}

void loop() {

#if SERIAL_MODE == 1
  float temperature = eeprom_read_float(0);
  sSerial.println(temperature);
#elif SERIAL_MODE == 2
  float temperature = eeprom_read_float(0);
  Serial.println(temperature);
#else
  digitalWrite(1, HIGH);
   float temperature;
   getTemperature(&temperature);
   float pressure;
   getPressure(&pressure);
   float seaLevelPressure = SENSORS_PRESSURE_SEALEVELHPA;
   float altitude = pressureToAltitude(seaLevelPressure, pressure/100);

   eeprom_write_float(0, altitude);

  digitalWrite(1, LOW);

#endif
  delay(1000);
}
