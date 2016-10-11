/* This file contains code from Adafuit's BMP085 pressure sensor library.
 *  I modified it to make it work on a digispark board, powerd by an AtTiny85
 *  Original License below:
 */

/***************************************************************************
  This is a library for the BMP085 pressure sensor
  Designed specifically to work with the Adafruit BMP085 or BMP180 Breakout 
  ----> http://www.adafruit.com/products/391
  ----> http://www.adafruit.com/products/1603
  These displays use I2C to communicate, 2 pins are required to interface.
  Adafruit invests time and resources providing this open source code,
  please support Adafruit andopen-source hardware by purchasing products
  from Adafruit!
  Written by Kevin Townsend for Adafruit Industries.  
  BSD license, all text above must be included in any redistribution
 ***************************************************************************/

 #include "TinyWireM.h"
 #define Wire TinyWireM


#define BMP085_USE_DATASHEET_VALS (0)


/*=========================================================================
    I2C ADDRESS/BITS
    -----------------------------------------------------------------------*/
    #define BMP085_ADDRESS                (0x77)
/*=========================================================================*/

/*=========================================================================
    REGISTERS
    -----------------------------------------------------------------------*/
    enum
    {
      BMP085_REGISTER_CAL_AC1            = 0xAA,  // R   Calibration data (16 bits)
      BMP085_REGISTER_CAL_AC2            = 0xAC,  // R   Calibration data (16 bits)
      BMP085_REGISTER_CAL_AC3            = 0xAE,  // R   Calibration data (16 bits)
      BMP085_REGISTER_CAL_AC4            = 0xB0,  // R   Calibration data (16 bits)
      BMP085_REGISTER_CAL_AC5            = 0xB2,  // R   Calibration data (16 bits)
      BMP085_REGISTER_CAL_AC6            = 0xB4,  // R   Calibration data (16 bits)
      BMP085_REGISTER_CAL_B1             = 0xB6,  // R   Calibration data (16 bits)
      BMP085_REGISTER_CAL_B2             = 0xB8,  // R   Calibration data (16 bits)
      BMP085_REGISTER_CAL_MB             = 0xBA,  // R   Calibration data (16 bits)
      BMP085_REGISTER_CAL_MC             = 0xBC,  // R   Calibration data (16 bits)
      BMP085_REGISTER_CAL_MD             = 0xBE,  // R   Calibration data (16 bits)
      BMP085_REGISTER_CHIPID             = 0xD0,
      BMP085_REGISTER_VERSION            = 0xD1,
      BMP085_REGISTER_SOFTRESET          = 0xE0,
      BMP085_REGISTER_CONTROL            = 0xF4,
      BMP085_REGISTER_TEMPDATA           = 0xF6,
      BMP085_REGISTER_PRESSUREDATA       = 0xF6,
      BMP085_REGISTER_READTEMPCMD        = 0x2E,
      BMP085_REGISTER_READPRESSURECMD    = 0x34
    };
/*=========================================================================*/

/*=========================================================================
    MODE SETTINGS
    -----------------------------------------------------------------------*/
    typedef enum
    {
      BMP085_MODE_ULTRALOWPOWER          = 0,
      BMP085_MODE_STANDARD               = 1,
      BMP085_MODE_HIGHRES                = 2,
      BMP085_MODE_ULTRAHIGHRES           = 3
    } bmp085_mode_t;
/*=========================================================================*/

/*=========================================================================
    CALIBRATION DATA
    -----------------------------------------------------------------------*/
    typedef struct
    {
      int16_t  ac1;
      int16_t  ac2;
      int16_t  ac3;
      uint16_t ac4;
      uint16_t ac5;
      uint16_t ac6;
      int16_t  b1;
      int16_t  b2;
      int16_t  mb;
      int16_t  mc;
      int16_t  md;
    } bmp085_calib_data;
/*=========================================================================*/

static volatile bmp085_calib_data _bmp085_coeffs;   // Last read accelerometer data will be available here
static uint8_t _bmp085Mode = BMP085_MODE_ULTRAHIGHRES;



/**************************************************************************/
/*!
    @brief  Writes an 8 bit value over I2C
*/
/**************************************************************************/
static void writeCommand(byte reg, byte value)
{
  Wire.beginTransmission((uint8_t)BMP085_ADDRESS);
  Wire.write(reg);
  Wire.write(value);
  Wire.endTransmission();
}

/**************************************************************************/
/*!
    @brief  Reads an 8 bit value over I2C
*/
/**************************************************************************/
static uint8_t read8(byte reg)
{
  uint8_t value;
  Wire.beginTransmission((uint8_t)BMP085_ADDRESS);
  Wire.write(reg);
  Wire.endTransmission();
  
  Wire.requestFrom((uint8_t)BMP085_ADDRESS, (byte)1);
  value = Wire.read();
  Wire.endTransmission();
  return value;
}

/**************************************************************************/
/*!
    @brief  Reads a 16 bit value over I2C
*/
/**************************************************************************/
static uint16_t read16(byte reg)
{
  uint16_t value;
  Wire.beginTransmission((uint8_t)BMP085_ADDRESS);
  Wire.write(reg);
  Wire.endTransmission();
  
  Wire.requestFrom((uint8_t)BMP085_ADDRESS, (byte)2);
  value = (Wire.read() << 8) | Wire.read();
  Wire.endTransmission();
  return value;
}

/**************************************************************************/
/*!
    @brief  Reads a signed 16 bit value over I2C
*/
/**************************************************************************/
static int16_t readS16(byte reg)
{
  return (int16_t)read16(reg);
}

static void readCoefficients(void)
{
  #if BMP085_USE_DATASHEET_VALS
    _bmp085_coeffs.ac1 = 408;
    _bmp085_coeffs.ac2 = -72;
    _bmp085_coeffs.ac3 = -14383;
    _bmp085_coeffs.ac4 = 32741;
    _bmp085_coeffs.ac5 = 32757;
    _bmp085_coeffs.ac6 = 23153;
    _bmp085_coeffs.b1  = 6190;
    _bmp085_coeffs.b2  = 4;
    _bmp085_coeffs.mb  = -32768;
    _bmp085_coeffs.mc  = -8711;
    _bmp085_coeffs.md  = 2868;
    _bmp085Mode        = 0;
  #else
    _bmp085_coeffs.ac1 = readS16(BMP085_REGISTER_CAL_AC1);
    _bmp085_coeffs.ac2 = readS16(BMP085_REGISTER_CAL_AC2);
    _bmp085_coeffs.ac3 = readS16(BMP085_REGISTER_CAL_AC3);
    _bmp085_coeffs.ac4 = read16(BMP085_REGISTER_CAL_AC4);
    _bmp085_coeffs.ac5 = read16(BMP085_REGISTER_CAL_AC5);
    _bmp085_coeffs.ac6 = read16(BMP085_REGISTER_CAL_AC6);
    _bmp085_coeffs.b1 = readS16(BMP085_REGISTER_CAL_B1);
    _bmp085_coeffs.b2 = readS16(BMP085_REGISTER_CAL_B2);
    _bmp085_coeffs.mb = readS16(BMP085_REGISTER_CAL_MB);
    _bmp085_coeffs.mc = readS16(BMP085_REGISTER_CAL_MC);
    _bmp085_coeffs.md = readS16(BMP085_REGISTER_CAL_MD);
  #endif
}

int16_t getPressure() {
  Wire.beginTransmission(0x77);
  Wire.write(0xF4);
  Wire.write(0x34);
  Wire.endTransmission();

  delay(10);

  Wire.beginTransmission(0x77);
  Wire.write(0xF6);
  Wire.endTransmission();
  Wire.requestFrom(0x77, (byte)2);
  uint16_t pressure = (Wire.read() << 8) | Wire.read();
  Wire.endTransmission();
  return pressure;
}

static int readRawTemperature()
{
  int32_t temperature;
  #if BMP085_USE_DATASHEET_VALS
    temperature = 27898;
  #else
    uint16_t t;
    writeCommand(BMP085_REGISTER_CONTROL, BMP085_REGISTER_READTEMPCMD);
    delay(5);
    t = read16(BMP085_REGISTER_TEMPDATA);
    temperature = t;
  #endif
}

float compensatedTemp() {
  int32_t UT,B5;     // following ds convention
  float t;

  UT = readRawTemperature();

  #if BMP085_USE_DATASHEET_VALS
    // use datasheet numbers!
    UT = 27898;
    _bmp085_coeffs.ac6 = 23153;
    _bmp085_coeffs.ac5 = 32757;
    _bmp085_coeffs.mc = -8711;
    _bmp085_coeffs.md = 2868;
  #endif

  int32_t X1 = (UT - (int32_t)_bmp085_coeffs.ac6) * ((int32_t)_bmp085_coeffs.ac5) >> 15;
  int32_t X2 = ((int32_t)_bmp085_coeffs.mc << 11) / (X1+(int32_t)_bmp085_coeffs.md);
  B5 =  X1 + X2;
  t = (B5+8) >> 4;
  t /= 10;
  return t;

}

int compensatedPress() {
  int32_t  ut = 0, up = 0, compp = 0;
  int32_t  x1, x2, b5, b6, x3, b3, p;
  uint32_t b4, b7;
  ut = readRawTemperature();
  up = getPressure();
  
}
