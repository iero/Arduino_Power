/*
  SD card datalogger
 	 
 */
#include <SPI.h>
#include <dht.h>
#include <SD.h>
#include <Wire.h>
#include "RTClib.h"

#define LOG_INTERVAL  5000 // mills between entries

#define dht22Pin 2  // DHT22 on digital pin 2 
dht DHT;            // DHT22 (temperature and humidity)
RTC_Millis rtc;     // Real Time Clock

File logFile;

void error(char *str) {
  //Serial.print("error: ");
  //Serial.println(str);
  while(1);
}

void setup() {
  Serial.begin(9600);
  while (!Serial) { ; }  // wait for serial port to connect. Needed for Leonardo only

  // RTC setup
  rtc.begin(DateTime(F(__DATE__), F(__TIME__)));

  //Serial.print("Initializing SD card...");
  pinMode(SS, OUTPUT);
  
  // see if the card is present and can be initialized:
  if (!SD.begin(10, 11, 12, 13)) {
  //  Serial.println("Card failed, or not present");
    while (1) ;    // don't do anything more:
  }
  //Serial.println("Card initialized.");
    
  // fetch the time for file name
  DateTime now = rtc.now();
  String dirNameString = "";
  String fileNameString = "";

  dirNameString += now.year();
  if (now.month() < 10) {
    dirNameString += "0" ; 
    dirNameString += now.month();
  } else dirNameString += now.month();
  if (now.day() < 10) {
    dirNameString += "0" ; 
    dirNameString += now.day();
  } else dirNameString += now.day();
  
  char direname[dirNameString.length()+1];
  dirNameString.toCharArray(direname, sizeof(direname));
  if ( ! SD.exists(direname)) {
    SD.mkdir(direname) ;
    //Serial.println("Directory "+dirNameString+" created");
  } //else Serial.println("Directory "+dirNameString+" exists");
  
  fileNameString += dirNameString+"/";
  
  if (now.hour() < 10) {
    fileNameString += "0" ; 
    fileNameString += now.hour();
  } else fileNameString += now.hour();
  if (now.minute() < 10) {
    fileNameString += "0" ; 
    fileNameString += now.minute();
  } else fileNameString += now.minute();
  if (now.second() < 10) {
    fileNameString += "0" ; 
    fileNameString += now.second();
  } else fileNameString += now.second();
    
  // create a new file
  fileNameString += ".csv";
  char filename[fileNameString.length()+1];
  fileNameString.toCharArray(filename, sizeof(filename));
  logFile = SD.open(filename, FILE_WRITE); 
  Serial.println("File "+fileNameString+" created");
  
  if (! logFile) {
    error("couldnt create file");
  }
}

void loop() {  
  // delay for the amount of time we want between readings
  delay((LOG_INTERVAL -1) - (millis() % LOG_INTERVAL));

  // Read DHT22 sensor
  int chk = DHT.read22(dht22Pin);
  
  //Serial.print("Temperature : ");    
  //Serial.println(DHT.temperature);
  //Serial.print("Humidite : ");    
  //Serial.println(DHT.humidity);
  
  // fetch the time
  DateTime now = rtc.now();
    
  // make a string for assembling the data to log:
  String dataString = "";

  // log time
  dataString += now.year();
  dataString += "/";
  if (now.month() < 10) {
    dataString += "0" ; 
    dataString += now.month();
  } else dataString += now.month();
  dataString += "/";
  if (now.day() < 10) {
    dataString += "0" ; 
    dataString += now.day();
  } else dataString += now.day();
  dataString += " ";
  if (now.hour() < 10) {
    dataString += "0" ; 
    dataString += now.hour();
  } else dataString += now.hour();
  dataString += ":";
  if (now.minute() < 10) {
    dataString += "0" ; 
    dataString += now.minute();
  } else dataString += now.minute();
  dataString += ":";
  if (now.second() < 10) {
    dataString += "0" ; 
    dataString += now.second();
  } else dataString += now.second();
    
  dataString += ",";

  // log dht22 sensor
  dataString += DHT.temperature;
  dataString += ",";
  dataString += DHT.humidity;

  // write log
  logFile.println(dataString);

  // print to the serial port too:
  Serial.println(dataString);
  
  logFile.flush();  
}


