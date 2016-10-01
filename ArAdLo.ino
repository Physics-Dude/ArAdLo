/*
 * ArAdLo
 * ArAdLo = Arduino Advanced Logger
 * 
 * By Physics_Dude (Mike H)
 * Visit thestufffwebuild.com for more
 */

/************************ Includes/Conf ************************/
// For reading available memory in log
#include <MemoryFree.h>

// Commons
#include <SPI.h>
#include <SD.h>
#include <Wire.h>

// GPS shield
#include <Adafruit_GPS.h>
#include <SoftwareSerial.h>
SoftwareSerial mySerial(8, 6);
Adafruit_GPS GPS(&mySerial);

// IR thermopile
#include <Adafruit_MLX90614.h>
Adafruit_MLX90614 mlx = Adafruit_MLX90614();

// UV Index
#include "Adafruit_SI1145.h"
Adafruit_SI1145 uv = Adafruit_SI1145();

// AM2315 humidity sensor
#include <Adafruit_AM2315.h>
Adafruit_AM2315 am2315;

// Analog thermistor
#define steinhartSTORPIN A0 // which analog pin to connect
#define steinhartSTORNOMINAL 20000 // resistance at 25 degrees C
#define TEMPERATURENOMINAL 25 // temp. for nominal resistance
#define NUMSAMPLES 5 // how many samples to take and average
#define BCOEFFICIENT 3892 // The beta coefficient of the steinhartstor
#define SERIESRESISTOR 10000 // the value of the 'other' resistor

// SD
#define chipSelect 10
char filename[12];

// Write the CSV header?
boolean header = true;

// Vars for setting interval time
unsigned long timer;
#define interval 1000 //ms between logs

// Keep track of howe many samples were done
unsigned long sample = 0;

// An LED used to show good GPS sat fix
// off = < 4 sats, blink = 4-5 sats, on = 6+ sats.
boolean GPSblinkState = LOW;
#define GPSblinkPin 5

void setup()
{
  // Set pin modes
  pinMode(chipSelect, OUTPUT); // For SD
  pinMode(GPSblinkPin, OUTPUT); // For GPS sat fix LED

  // RC PWM input for logging.
  pinMode(4, INPUT_PULLUP);

  // Option available to use 3.3v Aref for thermistor sense
  analogReference(EXTERNAL);

  // See if the card is present and can be initialized:
  if (!SD.begin(chipSelect)) {
    // SD failed
    return;
  }

  // Make a file
  strcpy(filename, "LOG00.csv");
  for (uint8_t i = 0; i < 100; i++) {
    filename[3] = '0' + i / 10;
    filename[4] = '0' + i % 10;
    // create if does not exist
    if (! SD.exists(filename)) {
      break;
    }
  }

  // IR thermopile
  mlx.begin();

  // UV index
  uv.begin();

  // AM2315 humidity sensor
  am2315.begin();

  // 9600 NMEA is the default baud rate for Adafruit MTK GPS's
  GPS.begin(9600);

  // Turn on RMC and GGA sentences
  GPS.sendCommand(PMTK_SET_NMEA_OUTPUT_RMCGGA);

  // Set the update rate
  GPS.sendCommand(PMTK_SET_NMEA_UPDATE_1HZ);

  // We cool?
  delay(1000);

  // Ask GPS for firmware version
  //mySerial.println(PMTK_Q_RELEASE);
}


void loop()
{
  /************************ Read GPS Output ************************/
  // Do over and over
  char c = GPS.read();
  if (GPS.newNMEAreceived()) {
    if (!GPS.parse(GPS.lastNMEA()))
      return;  // We can fail to parse a sentence
  }

  /************************ Logging section ************************/
  if (millis() - timer > interval) {
    timer = millis(); // reset the timer

    //////////////////////// Sensor Calculations ////////////////////////
    //Thermistor air temp
    int samples[NUMSAMPLES];
    int i;
    float average;
    float steinhart = 0;
    // Take N samples in a row, to average noise
    for (i = 0; i < NUMSAMPLES; i++) {
      samples[i] = analogRead(steinhartSTORPIN);
    }
    // Average the samples and apply calculations
    average = 0;
    for (i = 0; i < NUMSAMPLES; i++) {
      average += samples[i];
    }
    average /= NUMSAMPLES;
    average = 1023 / average - 1;                // convert value to resistance
    average = SERIESRESISTOR / average;          // convert value to resistance
    steinhart = average / steinhartSTORNOMINAL;     // (R/Ro)
    steinhart = log(steinhart);                  // ln(R/Ro)
    steinhart /= BCOEFFICIENT;                   // 1/B * ln(R/Ro)
    steinhart += 1.0 / (TEMPERATURENOMINAL + 273.15); // + (1/To)
    steinhart = 1.0 / steinhart;                 // Invert
    steinhart -= 273.15;                         // convert to C

    // Convert UVindev
    float uvi = uv.readUV() / 100.0;

    // Increment sample count
    sample++;

    // Convert knots (don't forget to change units in CSV header text)
    float spd = GPS.speed * 1.852;        //kmh
    //float spd = GPS.speed * 1.15077945; //mph

    //RC pin sense (needs work)
    int RCpin = pulseIn(4, HIGH, 45000); // Can delay program for up to 45ms
    int lastRCpin;
    if (RCpin < 900) RCpin = lastRCpin;
    lastRCpin == RCpin;

    // Count the stars and blink an LED
    byte sats = (int)GPS.satellites;
    if (sats >= 6) { // Show solid LED if there is a great fix
      GPSblinkState = HIGH;
    }
    else if (sats >= 4) { // Blink if there is a 3D fix
      if (GPSblinkState == LOW)
        GPSblinkState = HIGH;
      else
        GPSblinkState = LOW;
    }
    else { // No LED if no GPS fix
      GPSblinkState = LOW;
    }
    digitalWrite(GPSblinkPin, GPSblinkState);

    //////////////////////// Write to logfile ////////////////////////

    // Open the log file
    File logfile = SD.open(filename, FILE_WRITE);

    // Print the header on the first line once
    if (header == true) {
      logfile.println(F("ms,sample,,Time,Date,,Satellites,Fix,Quality,,Lat(deg),Lon(deg),Alt(m),,Speed(km/h),Heading(deg),,IRTemp(C),AirTemp(C),RH(%),UVIndex,,RXpin,FreeRAM(b)"));
      header = false; // Dont' do it again
    }

    // Uptime
    logfile.print(millis());
    logfile.print(',');
    // Sample number
    logfile.print(sample);
    logfile.print(F(",,")); // Space

    // GPS time
    logfile.print(GPS.hour, DEC);
    logfile.print(':');
    logfile.print(GPS.minute, DEC);
    logfile.print(':');
    logfile.print(GPS.seconds, DEC);
    logfile.print('.');
    logfile.print(GPS.milliseconds);
    logfile.print(',');

    // GPS date
    logfile.print(GPS.day, DEC);
    logfile.print('/');
    logfile.print(GPS.month, DEC);
    logfile.print('/');
    logfile.print(GPS.year, DEC);
    logfile.print(F(",,")); // Sapce

    // GPS sat count
    logfile.print(sats);
    logfile.print(',');

    // GPS is fix?
    logfile.print((int)GPS.fix);
    logfile.print(',');

    // GPS quality
    logfile.print((int)GPS.fixquality);
    logfile.print(F(",,")); // Space

    // GPS latitude
    logfile.print(GPS.latitudeDegrees, 7);
    logfile.print(',');

    // GPS longitude
    logfile.print(GPS.longitudeDegrees, 7);
    logfile.print(',');

    // GPS altitude
    logfile.print(GPS.altitude);
    logfile.print(F(",,")); // Space

    // GPS speed
    logfile.print(spd);
    logfile.print(',');

    // GPS heading
    logfile.print(GPS.angle);
    logfile.print(F(",,")); // Space

    // IR thermopile
    logfile.print(mlx.readObjectTempC());
    logfile.print(',');

    // Thermistor air temp
    logfile.print(steinhart);
    logfile.print(',');

    // AM2315 humidity
    logfile.print(am2315.readHumidity());
    logfile.print(',');

    // UV index
    logfile.print(uvi);
    logfile.print(F(",,")); // Space

    // RC PWM length (throttle ch1)
    logfile.print(RCpin);
    logfile.print(',');

    // RAM Debug
    logfile.print(freeMemory());
    logfile.print(',');

    // New line
    logfile.println();

    // Close the file:
    logfile.close();
  }
}




