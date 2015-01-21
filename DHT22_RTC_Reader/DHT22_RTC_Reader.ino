#include <dht.h>
#include <Wire.h>
#include "RTClib.h"

#define LOG_INTERVAL  5000 // mills between entries (reduce to take more/faster data)
// The analog pins that connect to the sensors
#define dht22Pin 2           // analog 1

RTC_DS1307 rtc;

dht DHT; // define DHT22 object

void setup(void) {
  Serial.begin(9600);
  Serial.println("Arduino started");
  Wire.begin();
  rtc.begin();
  //rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
}
  
void loop(void) {
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
   Serial.println(dataString);
}
