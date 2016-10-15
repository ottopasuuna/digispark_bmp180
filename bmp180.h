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

static bmp085_calib_data _bmp085_coeffs;   // Last read accelerometer data will be available here
static uint8_t _bmp085Mode = BMP085_MODE_ULTRAHIGHRES;



/**************************************************************************/
/*!
    @brief  Writes an 8 bit value over I2C
*/
/**************************************************************************/
static void writeCommand(byte reg, byte value)
{
  Wire.beginTransmission((uint8_t)BMP085_ADDRESS);

    Wire.write((uint8_t)reg);
    Wire.write((uint8_t)value);

  Wire.endTransmission();
}

/**************************************************************************/
/*!
    @brief  Reads an 8 bit value over I2C
*/
/**************************************************************************/
static void read8(byte reg, uint8_t *value)
{
  Wire.beginTransmission((uint8_t)BMP085_ADDRESS);

  Wire.write((uint8_t)reg);

  Wire.endTransmission();
  Wire.requestFrom((uint8_t)BMP085_ADDRESS, (byte)1);
    *value = Wire.read();
 // Wire.endTransmission();
}

/**************************************************************************/
/*!
    @brief  Reads a 16 bit value over I2C
*/
/**************************************************************************/
static void read16(byte reg, uint16_t *value)
{
  Wire.beginTransmission((uint8_t)BMP085_ADDRESS);
    Wire.write((uint8_t)reg);
  Wire.endTransmission();
  Wire.requestFrom((uint8_t)BMP085_ADDRESS, (byte)2);
    *value = (Wire.read() << 8) | Wire.read();
  //Wire.endTransmission();
}

/**************************************************************************/
/*!
    @brief  Reads a signed 16 bit value over I2C
*/
/**************************************************************************/
static void readS16(byte reg, int16_t *value)
{
  uint16_t i;
  read16(reg, &i);
  *value = (int16_t)i;
}

/**************************************************************************/
/*!
    @brief  Reads the factory-set coefficients
*/
/**************************************************************************/
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
    readS16(BMP085_REGISTER_CAL_AC1, &_bmp085_coeffs.ac1);
    readS16(BMP085_REGISTER_CAL_AC2, &_bmp085_coeffs.ac2);
    readS16(BMP085_REGISTER_CAL_AC3, &_bmp085_coeffs.ac3);
    read16(BMP085_REGISTER_CAL_AC4, &_bmp085_coeffs.ac4);
    read16(BMP085_REGISTER_CAL_AC5, &_bmp085_coeffs.ac5);
    read16(BMP085_REGISTER_CAL_AC6, &_bmp085_coeffs.ac6);
    readS16(BMP085_REGISTER_CAL_B1, &_bmp085_coeffs.b1);
    readS16(BMP085_REGISTER_CAL_B2, &_bmp085_coeffs.b2);
    readS16(BMP085_REGISTER_CAL_MB, &_bmp085_coeffs.mb);
    readS16(BMP085_REGISTER_CAL_MC, &_bmp085_coeffs.mc);
    readS16(BMP085_REGISTER_CAL_MD, &_bmp085_coeffs.md);
  #endif
}

/**************************************************************************/
/*!
*/
/**************************************************************************/
static void readRawTemperature(int32_t *temperature)
{
  #if BMP085_USE_DATASHEET_VALS
    *temperature = 27898;
  #else
    uint16_t t;
    writeCommand(BMP085_REGISTER_CONTROL, BMP085_REGISTER_READTEMPCMD);
    delay(5);
    read16(BMP085_REGISTER_TEMPDATA, &t);
    *temperature = t;
  #endif
}

/**************************************************************************/
/*!
*/
/**************************************************************************/
static void readRawPressure(int32_t *pressure)
{
  #if BMP085_USE_DATASHEET_VALS
    *pressure = 23843;
  #else
    uint8_t  p8;
    uint16_t p16;
    int32_t  p32;

    writeCommand(BMP085_REGISTER_CONTROL, BMP085_REGISTER_READPRESSURECMD + (_bmp085Mode << 6));
    switch(_bmp085Mode)
    {
      case BMP085_MODE_ULTRALOWPOWER:
        delay(5);
        break;
      case BMP085_MODE_STANDARD:
        delay(8);
        break;
      case BMP085_MODE_HIGHRES:
        delay(14);
        break;
      case BMP085_MODE_ULTRAHIGHRES:
      default:
        delay(26);
        break;
    }

    read16(BMP085_REGISTER_PRESSUREDATA, &p16);
    p32 = (uint32_t)p16 << 8;
    read8(BMP085_REGISTER_PRESSUREDATA+2, &p8);
    p32 += p8;
    p32 >>= (8 - _bmp085Mode);
    
    *pressure = p32;
  #endif
}

/**************************************************************************/
/*!
    @brief  Compute B5 coefficient used in temperature & pressure calcs.
*/
/**************************************************************************/
int32_t computeB5(int32_t ut) {
  int32_t X1 = (ut - (int32_t)_bmp085_coeffs.ac6) * ((int32_t)_bmp085_coeffs.ac5) >> 15;
  int32_t X2 = ((int32_t)_bmp085_coeffs.mc << 11) / (X1+(int32_t)_bmp085_coeffs.md);
  return X1 + X2;
}



 
/**************************************************************************/
/*!
    @brief  Setups the HW
*/
/**************************************************************************/
bool bmpinit(bmp085_mode_t mode)
{
  // Enable I2C
  Wire.begin();

  /* Mode boundary check */
  if ((mode > BMP085_MODE_ULTRAHIGHRES) || (mode < 0))
  {
    mode = BMP085_MODE_ULTRAHIGHRES;
  }

  /* Make sure we have the right device */
  uint8_t id;
  read8(BMP085_REGISTER_CHIPID, &id);
  if(id != 0x55)
  {
    return false;
  }

  /* Set the mode indicator */
  _bmp085Mode = mode;

  /* Coefficients need to be read once */
  readCoefficients();
    
  return true;
}

/**************************************************************************/
/*!
    @brief  Gets the compensated pressure level in kPa
*/
/**************************************************************************/
float getPressure()
{
  int32_t  ut = 0, up = 0, compp = 0;
  int32_t  x1, x2, b5, b6, x3, b3, p;
  uint32_t b4, b7;

  /* Get the raw pressure and temperature values */
  readRawTemperature(&ut);
  readRawPressure(&up);

  /* Temperature compensation */
  b5 = computeB5(ut);

  /* Pressure compensation */
  b6 = b5 - 4000;
  x1 = (_bmp085_coeffs.b2 * ((b6 * b6) >> 12)) >> 11;
  x2 = (_bmp085_coeffs.ac2 * b6) >> 11;
  x3 = x1 + x2;
  b3 = (((((int32_t) _bmp085_coeffs.ac1) * 4 + x3) << _bmp085Mode) + 2) >> 2;
  x1 = (_bmp085_coeffs.ac3 * b6) >> 13;
  x2 = (_bmp085_coeffs.b1 * ((b6 * b6) >> 12)) >> 16;
  x3 = ((x1 + x2) + 2) >> 2;
  b4 = (_bmp085_coeffs.ac4 * (uint32_t) (x3 + 32768)) >> 15;
  b7 = ((uint32_t) (up - b3) * (50000 >> _bmp085Mode));

  if (b7 < 0x80000000)
  {
    p = (b7 << 1) / b4;
  }
  else
  {
    p = (b7 / b4) << 1;
  }

  x1 = (p >> 8) * (p >> 8);
  x1 = (x1 * 3038) >> 16;
  x2 = (-7357 * p) >> 16;
  compp = p + ((x1 + x2 + 3791) >> 4);

  return compp;
}

/**************************************************************************/
/*!
    @brief  Reads the temperatures in degrees Celsius
*/
/**************************************************************************/
float getTemperature()
{
  int32_t UT, X1, X2, B5;     // following ds convention
  float t;

  readRawTemperature(&UT);

  #if BMP085_USE_DATASHEET_VALS
    // use datasheet numbers!
    UT = 27898;
    _bmp085_coeffs.ac6 = 23153;
    _bmp085_coeffs.ac5 = 32757;
    _bmp085_coeffs.mc = -8711;
    _bmp085_coeffs.md = 2868;
  #endif

  B5 = computeB5(UT);
  t = (B5+8) >> 4;
  t /= 10;

  return t;
}

/**************************************************************************/
/*!
    Calculates the altitude (in meters) from the specified atmospheric
    pressure (in hPa), and sea-level pressure (in hPa).
    @param  seaLevel      Sea-level pressure in hPa
    @param  atmospheric   Atmospheric pressure in hPa
*/
/**************************************************************************/
float pressureToAltitude(float seaLevel, float atmospheric)
{
  // Equation taken from BMP180 datasheet (page 16):
  //  http://www.adafruit.com/datasheets/BST-BMP180-DS000-09.pdf

  // Note that using the equation from wikipedia can give bad results
  // at high altitude.  See this thread for more information:
  //  http://forums.adafruit.com/viewtopic.php?f=22&t=58064
  
  return 44330.0 * (1.0 - pow(atmospheric / seaLevel, 0.1903));
}

/**************************************************************************/
/*!
    Calculates the altitude (in meters) from the specified atmospheric
    pressure (in hPa), and sea-level pressure (in hPa).  Note that this
    function just calls the overload of pressureToAltitude which takes
    seaLevel and atmospheric pressure--temperature is ignored.  The original
    implementation of this function was based on calculations from Wikipedia
    which are not accurate at higher altitudes.  To keep compatibility with
    old code this function remains with the same interface, but it calls the
    more accurate calculation.
    @param  seaLevel      Sea-level pressure in hPa
    @param  atmospheric   Atmospheric pressure in hPa
    @param  temp          Temperature in degrees Celsius
*/
/**************************************************************************/
float pressureToAltitude(float seaLevel, float atmospheric, float temp)
{
  return pressureToAltitude(seaLevel, atmospheric);
}

