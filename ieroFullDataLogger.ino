/*
  SD card datalogger
 	 
 */
#include <SPI.h>
#include <dht.h>
#include <SD.h>
#include <Wire.h>
#include "RTClib.h"

#define LOG_INTERVAL  5000 // mills between entries (reduce to take more/faster data)

#define dht22Pin 2  // analog 2 pin 
dht DHT; // define DHT22 object

RTC_DS1307 rtc; // define the Real Time Clock object

File logFile;

void error(char *str) {
  Serial.print("error: ");
  Serial.println(str);
  while(1);
}

void setup() {
  // Open serial communications and wait for port to open:
  Serial.begin(9600);
  while (!Serial) { ; }
    // wait for serial port to connect. Needed for Leonardo only

 // RTC setup
 Wire.begin();
 rtc.begin();
  if (! rtc.isrunning()) {
    Serial.println("RTC is NOT running!");
    // following line sets the RTC to the date & time this sketch was compiled
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  }

  Serial.print("Initializing SD card...");
  pinMode(SS, OUTPUT);
  
  // see if the card is present and can be initialized:
  if (!SD.begin(10, 11, 12, 13)) {
    Serial.println("Card failed, or not present");
    // don't do anything more:
    while (1) ;
  }
  Serial.println("card initialized.");
    
  // create a new file
  char filename[] = "log00.CSV";
  for (uint8_t i = 0; i < 100; i++) {
    filename[3] = i/10 + '0';
    filename[4] = i%10 + '0';
    if (! SD.exists(filename)) {
      // only open a new file if it doesn't exist
      Serial.print(filename);
      Serial.println(" created.");
      logFile = SD.open(filename, FILE_WRITE); 
      break;  // leave the loop!
    }
  }
  
  if (! logFile) {
    error("couldnt create file");
  }
  
}

void loop() {  
  // delay for the amount of time we want between readings
  delay((LOG_INTERVAL -1) - (millis() % LOG_INTERVAL));

  // Read DHT22 sensor
  //uint32_t start = micros(); 
  int chk = DHT.read22(dht22Pin);
  //uint32_t stop = micros();
  
  Serial.print("Temperature : ");    
  Serial.println(DHT.temperature);
  Serial.print("Humidite : ");    
  Serial.println(DHT.humidity);
  
  // fetch the time
  DateTime now = rtc.now();
    
  // make a string for assembling the data to log:
  String dataString = "";

  // log time
  dataString += now.year();
  dataString += "/";
  dataString += now.month();
  dataString += "/";
  dataString += now.day();
  dataString += " ";
  dataString += now.hour();
  dataString += ":";
  dataString += now.minute();
  dataString += ":";
  dataString += now.second();
  dataString += ",";

  // log dht22 sensor
  dataString += DHT.temperature;
  dataString += ",";
  dataString += DHT.humidity;

  // write log
  logFile.println(dataString);

  // print to the serial port too:
  Serial.println(dataString);
  
  // The following line will 'save' the file to the SD card after every
  // line of data - this will use more power and slow down how much data
  // you can read but it's safer! 
  // If you want to speed up the system, remove the call to flush() and it
  // will save the file only every 512 bytes - every time a sector on the 
  // SD card is filled with data.
  logFile.flush();
  
}


