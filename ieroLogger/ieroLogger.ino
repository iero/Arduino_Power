
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
// for LCD
#include <LiquidCrystal_I2C.h>

#define LOG_INTERVAL  15000  // mills between entries
#define DHT22PIN 8          // DHT22 on digital pin 8 
#define SCTPIN 2            // SCT on analog pin 2
#define CALIBSCT 30         // 30A max SCT
#define SERIAL 0            // 1 for serial and sd card, 0 for sd only
#define FRENCHTOUCH 1       // Replace . by , in values
#define SEPARATOR ';'      // Separator for CSV. ; is good for Numbers.

// objects creation

dht DHT;             // DHT22 (temperature and humidity)
RTC_DS1307 rtc;      // real time clock
EnergyMonitor sct;   // SCT 
LiquidCrystal_I2C lcd(0x20,16,2); // LCD

File logFile;
int loopCount=0;

// function def

String getCompleteDate();
String getDate(boolean reverse);
String getHour(boolean separator);
File createLogFile();
void printLog();
void rotateLog();

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
  
  // LCD
  lcd.init();                      // initialize the lcd 
  lcd.backlight();
  lcd.print("Hello world");
  
  pinMode(7,OUTPUT);
   
  sct.current(SCTPIN, CALIBSCT);

  if (! rtc.isrunning()) {
    printLog("RTC is NOT running!");
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  }
  
  #if SERIAL
    while (!Serial) { ; }  // wait for serial port to connect. Needed for Leonardo only
  #endif
  
  //Serial.print("Initializing SD card...");
  pinMode(SS, OUTPUT);
  
    // see if the card is present and can be initialized:
  if (!SD.begin(10, 11, 12, 13)) {
    printLog("Card failed, or not present");
    while (1) ;    // don't do anything more:
  }
  printLog("Card initialized.");
    
  logFile=createLogFile();
  logFile.println("DateTime;Temperature;Humidity;Irms;Power");
}

void loop () {
  // delay for the amount of time we want between readings
  delay((LOG_INTERVAL -1) - (millis() % LOG_INTERVAL));
  
  rotateLog();
  
  //print to the serial port too:
  #if SERIAL
    if (loopCount%10 == 0)
      Serial.println("   Date      Time  ,Temp, Humid,Irms,Pow") ;
  #endif
  
  readSensors();
    
  loopCount++;
  logFile.flush();  
}

String getCompleteDate() {
  String dataString = "";
  dataString += getDate(false);
  dataString += " " ;
  dataString += getHour(true);
 return dataString; 
}

String getDate(boolean reverse) {
  DateTime now = rtc.now();
  
  String dayString="";
  if (now.day() < 10) {
    dayString += "0" ; 
    dayString += now.day();
  } else dayString += now.day();

  String monthString="";
  if (now.month() < 10) {
    monthString += "0" ; 
    monthString += now.month();
  } else monthString += now.month();

  String dataString = "";
  if (!reverse) {
    dataString += dayString; 
    dataString += "/";
    dataString += monthString;
    dataString += "/";
    dataString += now.year();
  } else {
    dataString += now.year(); 
    dataString += monthString;
    dataString += dayString;
  }
  return dataString;
}

String getHour(boolean separator) {
  DateTime now = rtc.now();
  
  String dataString = "";
  if (now.hour() < 10) {
    dataString += "0" ; 
    dataString += now.hour();
  } else dataString += now.hour();
  if (separator) dataString += ":";
  if (now.minute() < 10) {
    dataString += "0" ; 
    dataString += now.minute();
  } else dataString += now.minute();
  if (separator) dataString += ":";
  if (now.second() < 10) {
    dataString += "0" ; 
    dataString += now.second();
  } else dataString += now.second();
}

File createLogFile() {
 // fetch the time for file name
  DateTime now = rtc.now();
  String dirNameString = "";
  String fileNameString = "";

  dirNameString=getDate(true);
  
  char direname[dirNameString.length()+1];
  dirNameString.toCharArray(direname, sizeof(direname));
  if ( ! SD.exists(direname)) {
    SD.mkdir(direname) ;
    //Serial.println("Directory "+dirNameString+" created");
  } //else Serial.println("Directory "+dirNameString+" exists");
  
  fileNameString += dirNameString+"/";
  fileNameString += getHour(false);
    
  // create a new file
  fileNameString += ".csv";
  char filename[fileNameString.length()+1];
  fileNameString.toCharArray(filename, sizeof(filename));
  return SD.open(filename, FILE_WRITE); 
  printLog("File created : "+fileNameString);
}

void printLog(String message) {
  #if SERIAL
    Serial.println(message);  
  #endif
  lcd.clear();
  lcd.print(message);
}

void rotateLog() {
  DateTime now = rtc.now();
  if (now.hour()==0 && now.minute()==0 && now.second() < 20)
    createLogFile();
  printLog("It's a new day");
}

void readSensors() {
  lcd.clear();
    // make a string for assembling the data to log:
  String dataString = "";

  // log time
  dataString += getCompleteDate();    
  dataString += SEPARATOR;

  digitalWrite(7, HIGH);   // turn the LED on (HIGH is the voltage level)
  delay(1000);              // wait for a second
  DHT.read22(DHT22PIN);
    // log dht22 sensor
  dataString += DHT.temperature;
  dataString += SEPARATOR;
  dataString += DHT.humidity;
  dataString += SEPARATOR;

  //lcd.setCursor(9, 0);
  //lcd.print(DHT.temperature);
  
   // log sct sensor
  float irms = sct.calcIrms(1480);  // 1480 samples taken
  float power = irms*230;
  if (loopCount > 0) {
    dataString += irms;
    dataString += SEPARATOR;  
    dataString += power ;
  } else {
    dataString += "0.00";
    dataString += SEPARATOR;
    dataString += "0.00";
  }

  delay(1000);              // wait for a second 
  digitalWrite(7, LOW);    // turn the LED off by making the voltage LOW

  // Some french touch
  #if FRENCHTOUCH
    dataString.replace(".", ",");
  #endif
  
  // write log
  logFile.println(dataString);

  // print to the serial port too:
  #if SERIAL
    Serial.println(dataString);
  #endif
}
  


