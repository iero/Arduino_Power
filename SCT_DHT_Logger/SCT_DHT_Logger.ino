
// Log SCT and DHT values values on SD card. 

// For RTC clock reading
#include <Wire.h>
#include "RTClib.h"
// for DHT22
#include <dht.h>
// For SD logging
#include <SD.h>
#include <SPI.h>
// for SCT logging
#include "EmonLib.h"

#define LOG_INTERVAL  5000  // mills between entries
#define DHT22PIN 8          // DHT22 on digital pin 8 
#define SCTPIN 2            // SCT on analog pin 2

#define CALIBSCT 30         // 30A max SCT

#define SERIAL 0            // 1 for serial and sd card, 0 for sd only

dht DHT;             // DHT22 (temperature and humidity)
RTC_DS1307 rtc;      // real time clock
EnergyMonitor sct;   // SCT 

File logFile;
int loopCount=0;

void setup () {
  #if SERIAL
    Serial.begin(9600);
  #endif

  #ifdef AVR
    Wire.begin();
  #else
    Wire1.begin(); // Shield I2C pins connect to alt I2C bus on Arduino Due
  #endif
  
  rtc.begin();
  
  pinMode(7,OUTPUT);
   
  sct.current(SCTPIN, CALIBSCT);

  if (! rtc.isrunning()) {
    #if SERIAL
      Serial.println("RTC is NOT running!");
    #endif
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  }
  
  #if SERIAL
    while (!Serial) { ; }  // wait for serial port to connect. Needed for Leonardo only
  #endif
  
  //Serial.print("Initializing SD card...");
  pinMode(SS, OUTPUT);
  
    // see if the card is present and can be initialized:
  if (!SD.begin(10, 11, 12, 13)) {
    #if SERIAL
      Serial.println("Card failed, or not present");
    #endif
    while (1) ;    // don't do anything more:
  }
    #if SERIAL
      Serial.println("Card initialized.");
    #endif
    
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
  #if SERIAL
    Serial.println("File "+fileNameString+" created");
  #endif
}

void loop () {
  // delay for the amount of time we want between readings
  delay((LOG_INTERVAL -1) - (millis() % LOG_INTERVAL));
  
  // fetch the time
  DateTime now = rtc.now();

  //print to the serial port too:
  #if SERIAL
    if (loopCount%10 == 0)
      Serial.println("   Date      Time  ,Temp, Humid,Irms,Pow") ;
  #endif
  
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

  digitalWrite(7, HIGH);   // turn the LED on (HIGH is the voltage level)
  delay(1000);              // wait for a second
  DHT.read22(DHT22PIN);
    // log dht22 sensor
  dataString += DHT.temperature;
  dataString += ",";
  dataString += DHT.humidity;
  dataString += ",";
  
   // log sct sensor
  float irms = sct.calcIrms(1480);  // 1480 samples taken
  float power = irms*230;
  if (loopCount > 0) {
    dataString += irms;
    dataString += ",";  
    dataString += power ;
  } else
    dataString += "0.00,0.00";

  delay(1000);              // wait for a second 
  digitalWrite(7, LOW);    // turn the LED off by making the voltage LOW

  // write log
  logFile.println(dataString);

  // print to the serial port too:
  #if SERIAL
    Serial.println(dataString);
  #endif
  
  loopCount++;
  logFile.flush();  
}
