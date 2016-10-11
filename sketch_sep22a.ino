
#define SERIAL_MODE 1


#include <avr/eeprom.h>

#if SERIAL_MODE == 1
  #include <SoftSerial.h>
  const int Rx = 3;
  const int Tx = 4;
  SoftSerial sSerial = SoftSerial(Rx, Tx);
#else
  #include <Adafruit_Sensor.h>
  #include <Adafruit_BMP085_U.h>
  //#include "bmp180.h"

  Adafruit_BMP085_Unified bmp = Adafruit_BMP085_Unified(10085);
 #endif


void setup() {
  // put your setup code here, to run once:
  #if SERIAL_MODE == 1
  sSerial.begin(9600);
  #elif SERIAL_MODE == 2
  Serial.begin(9600);
  #elif SERIAL_MODE == 0
  // Enable I2C
  bmp.begin();
  #endif

}

void loop() {

#if SERIAL_MODE == 1
  float temperature = eeprom_read_float(0);
  sSerial.println(temperature);
  delay(1000);
#elif SERIAL_MODE == 2
  float temperature = eeprom_read_float(0);
  Serial.println(temperature);
  delay(1000);

#else
  digitalWrite(1, HIGH);
  sensors_event_t event;
  bmp.getEvent(&event);
 
  /* Display the results (barometric pressure is measure in hPa) */
  if (event.pressure)
  {
    /* Display atmospheric pressue in hPa */
    float temperature;
    bmp.getTemperature(&temperature);
    eeprom_write_float(0, temperature);

  }
  digitalWrite(1, LOW);
  delay(1000);
#endif

}
