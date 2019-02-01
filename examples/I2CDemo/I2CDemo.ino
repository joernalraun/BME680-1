/*! @file I2CDemo.ino


@section I2CDemo_intro_section Description

Example program for using the Bosch BME680 sensor. The sensor measures temperature, pressure and humidity and  is
described at https://www.bosch-sensortec.com/bst/products/all_products/BME680. The datasheet is available from Bosch
at https://ae-bst.resource.bosch.com/media/_tech/media/datasheets/BST-BME680_DS001-11.pdf \n\n

The most recent version of the BME680 library is available at https://github.com/SV-Zanshin/BME680 and the 
documentation of the library as well as example programs are described in the project's wiki pages located at 
https://github.com/SV-Zanshin/BME680/wiki. \n\n

The BME680 is a very small package so it is unlikely for an Arduino hobbyist to play around with directly, the 
hardware used to develop this library is a breakout board from AdaFruit which is well-documented at
https://learn.adafruit.com/adafruit-BME680-humidity-barometric-pressure-temperature-sensor-breakout \n\n

This example program initializes the BME680 to use I2C for communications. The library does not using floating
point mathematics to save on computation space and time, the values for Temperature, Pressure and Humidity are
returned in deci-units, e.g. a Temperature reading of "2731" means "27.31" degrees Celsius. The display in the 
example program uses floating point for demonstration purposes only.  Note that the temperature reading is 
generally higher than the ambient temperature due to die and PCB temperature and self-heating of the element.\n\n

The pressure reading needs to be adjusted for altitude to get the adjusted pressure reading. There are numerous
sources on the internet for formula converting from standard sea-level pressure to altitude, see the data sheet
for the BME180 on page 16 of http://www.adafruit.com/datasheets/BST-BMP180-DS000-09.pdf. Rather than put a
floating-point function in the library which may not be used but which would use space, an example altitude
computation function has been added to this example program to show how it might be done.

@section I2CDemolicense License

This program is free software: you can redistribute it and/or modify it under the terms of the GNU General
Public License as published by the Free Software Foundation, either version 3 of the License, or (at your
option) any later version. This program is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details. You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.

@section I2CDemoauthor Author

Written by Arnd\@SV-Zanshin

@section I2CDemoversions Changelog

Version | Date       | Developer           | Comments
------- | ---------- | ------------------- | --------
1.0.1   | 2019-01-30 | https://github.com/SV-Zanshin | Removed old comments
1.0.1   | 2019-01-26 | https://github.com/SV-Zanshin | Issue #3 - convert documentation to Doxygen
1.0.0b  | 2018-06-30 | https://github.com/SV-Zanshin | Cloned from original BME280 program
*/
#include "Zanshin_BME680.h" // Include the BME680 Sensor library
/*******************************************************************************************************************
** Declare all program constants                                                                                  **
*******************************************************************************************************************/
const uint32_t SERIAL_SPEED = 115200; ///< Set the baud rate for Serial I/O

/*******************************************************************************************************************
** Declare global variables and instantiate classes                                                               **
*******************************************************************************************************************/
BME680_Class BME680; ///< Create an instance of the BME680

/*!
* @brief     This converts a pressure measurement into a height in meters
* @details   The corrected sea-level pressure can be passed into the function if it is know, otherwise the standard 
*            atmospheric pressure of 1013.25hPa is used (see https://en.wikipedia.org/wiki/Atmospheric_pressure
* @param[in] seaLevel Sea-Level pressure in millibars
* @return    floating point altitude in meters.
*/
float altitude(const float seaLevel=1013.25) 
{
  static float Altitude;
  int32_t temp, hum, press, gas;
  BME680.getSensorData(temp,hum,press,gas); // Get the most recent values from the device
  Altitude = 44330.0*(1.0-pow(((float)press/100.0)/seaLevel,0.1903)); // Convert into altitude in meters
  return(Altitude);
} // of method altitude()


/*!
    @brief    Arduino method called once at startup to initialize the system
    @details  This is an Arduino IDE method which is called first upon boot or restart. It is only called one time
              and then control goes to the main "loop()" method, from which control never returns
    @return   void
*/
void setup()
{
  Serial.begin(SERIAL_SPEED); // Start serial port at Baud rate
  #ifdef  __AVR_ATmega32U4__  // If this is a 32U4 processor, then wait 3 seconds to initialize USB
    delay(3000);
  #endif
  Serial.println(F("Starting I2CDemo example program for BME680"));
  Serial.print(F("- Initializing BME680 sensor\n"));
  while (!BME680.begin(I2C_STANDARD_MODE)) // Start BME680 using I2C protocol
  {
    Serial.println(F("-  Unable to find BME680. Waiting 3 seconds."));
    delay(3000);
  } // of loop until device is located
  Serial.println(F("- Setting 16x oversampling for all sensors"));
  BME680.setOversampling(TemperatureSensor,Oversample16); // Use enumerated type values
  BME680.setOversampling(HumiditySensor,   Oversample16);
  BME680.setOversampling(PressureSensor,   Oversample16);
  Serial.println(F("- Setting IIR filter to a value of 4 samples"));
  BME680.setIIRFilter(IIR4);
  Serial.println(F("- Setting gas measurement to 320\xC2\xB0\C for 150ms"));
  BME680.setGas(320,150); // 320�c for 150 milliseconds
  Serial.println();
} // of method setup()

/*!
    @brief    Arduino method for the main program loop
    @details  This is the main program for the Arduino IDE, it is an infinite loop and keeps on repeating. 
    @return   void
*/
void loop() 
{
  static uint8_t loopCounter = 0;
  static int32_t temperature, humidity, pressure, gas;     // Variable to store readings
  BME680.getSensorData(temperature,humidity,pressure,gas); // Get most recent readings
  Serial.print(temperature/100.0,2);                       // Temperature in deci-degrees
#ifdef ESP32
  Serial.print(F(" ")); // Esp32 compiler doesn't liked escaped string
#else
  Serial.print(F("\xC2\xB0\C "));                          // Representation of the � symbol
#endif
  Serial.print(humidity/1000.0,2);                         // Humidity in milli-percent
  Serial.print(F("%Hum "));
  Serial.print(pressure/100.0,2);                          // Pressure in Pascals
  Serial.print(F("hPa "));
  Serial.print(altitude(),2);
  Serial.print(F("m "));
  Serial.print(gas/100.0,2);
  Serial.println(F("mOhm"));
  delay(5000);
} // of method loop()
