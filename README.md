# ArAdLo
**ArAdLo** = **Ar**duino **Ad**vanced **Lo**gger

ArAdLo is a highly configurable atmospheric data logging sketch that lets you use any ATmega328-family Arduino (or larger) to interface with, and log data from a slew of both analog and digital I2C sensors. 

The project is currently focused for multicopter ("drone") use in climate modelling and mapping by logging geo-specific atmospheric data to a CSV file on an SD card from, for example, the following sensors:
- AM2315 Temperature/Humidity Sensor
- MLX90614 Infrared Temperature Sensor
- SI1145 UV Index / IR / Visible Light sensor
- 20kR thermistor (for faster temperature response)
- Adafruit Ultimate GPS shield (MTK3332 GPS chipset and MicroSD slot)
- RX pin (Reads the PWM value from an RC radio receiver)

**Library Requirements** (includes sensors above)**:**
- #include <MemoryFree.h> //Used for debug
- #include <SPI.h> //Common
- #include <SD.h> //Common
- #include <Wire.h> //Common
- #include <Adafruit_GPS.h> //GPS
- #include <SoftwareSerial.h> //GPS
- #include <Adafruit_MLX90614.h> //IR Thermopile
- #include "Adafruit_SI1145.h" //UV Index sensor
- #include <Adafruit_AM2315.h> //temp/humidity sensor

**Wiring:**
todo 

**User Modification:**
All sections of the sketch are commented thoroughly and can be omitted or added to without much hassle. Sections tied to an "//IR thermopile" comment for example can be omitted, and the CSV header line can be modified to fit. It's encouraged that you first tinker with your sensor's example sketches, then copy over the necessary lines of code for logging. 

**CSV Use:**
CSVs or Comma Separated Value files are an easy way to format plan text data into tables or spread sheets. The CSV's line breaks are analogous to a table's rows. Likewise, the commas delimit the columns in a table. 


**Data samples:**
Plain text CSV output example (first line is the included header stored on the Arduino):
ms,sample,,Time,Date,,Satellites,Fix,Quality,,Lat(deg),Lon(deg),Alt(m),,Speed(km/h),Heading(deg),,IRTemp(C),AirTemp(C),RH(%),UVIndex,,RXpin,FreeRAM(b)
33652,33,,51:00.0,8/3/2016,,7,1,1,,26.2692375,-80.2232971,3.2,,0.19,25.11,,23.35,23.48,59.7,0.41,,984,237
34661,34,,51:01.0,8/3/2016,,7,1,1,,26.2692375,-80.2232971,3.2,,0.31,39.82,,23.45,23.48,59.7,0.4,,988,237
35651,35,,51:02.0,8/3/2016,,7,1,1,,26.2692375,-80.2232971,3.2,,0.26,65.82,,23.33,23.48,59.7,0.4,,984,237
36660,36,,51:03.0,8/3/2016,,7,1,1,,26.2692375,-80.2232971,3.2,,0.28,94.58,,23.29,23.48,59.7,0.4,,984,237

Log output example (CSV in Excel):
![CSV in MS Excel](http://i.imgur.com/eclWjhg.png "CSV in MS Excel")

Manual tricopter survey of IR temperature data plotted to GPS position:
![CSV in MS Excel](http://i.imgur.com/XH3HwEq.png "CSV in MS Excel")

**ToDo:**
- Make a Fritzing drawing of circuit.
- Spruce up comments.
- Improve RXPin section.
- Something with free ram. 
