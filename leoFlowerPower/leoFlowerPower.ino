// Automated water system

// for DHT22
//#include "DHT.h"

// For RTC clock reading
#include <Wire.h>
#include "RTClib.h"

#define LOOP 5000

#define LUM_START_HOUR 8
#define LUM_START_MIN 30

#define LUM_STOP_HOUR 19
#define LUM_STOP_MIN 0

#define MOIST_MIN 750 // Min Moisture to start pump
#define MOIST_SEC 500 // To detect if moist sensor is out
#define MOIST_TIMER 5000 // Waiting time to power moisture

#define WATER_TIME 2  // seconds to pump

#define ledPin 7

// Temp
//#define dht22Pin 2

// Light
#define lightPin 3

// Water Pump
#define waterPin 4

// Moisture
#define voltageFlipPin1 8
#define voltageFlipPin2 9
#define sensorPin A0

#define DHTTYPE DHT22
//DHT dht(dht22Pin, DHTTYPE);             // DHT22 (temperature and humidity)

RTC_DS1307 rtc;      // real time clock

void setSensorPolarity(boolean flip) {
  if (flip) {
    digitalWrite(voltageFlipPin1, HIGH);
    digitalWrite(voltageFlipPin2, LOW);
  } else {
    digitalWrite(voltageFlipPin1, LOW);
    digitalWrite(voltageFlipPin2, HIGH);
  }
}

void setup () {
  Serial.begin(9600);

//  #ifdef AVR
//    Wire.begin();
//  #else
//    Wire1.begin(); // Shield I2C pins connect to alt I2C bus on Arduino Due
//  #endif
  
  rtc.begin();
//  dht.begin();

  pinMode(voltageFlipPin1, OUTPUT);
  pinMode(voltageFlipPin2, OUTPUT);
  pinMode(sensorPin, INPUT);  
    
  pinMode(lightPin, OUTPUT);
  pinMode(waterPin, OUTPUT);
    
  pinMode(ledPin,OUTPUT);

  digitalWrite(lightPin, LOW);
  digitalWrite(waterPin, LOW);
//  digitalWrite(waterPin, HIGH);

  if (! rtc.begin()) {
    Serial.println("Couldn't find RTC");
    while (1);
  }
   
  if (! rtc.isrunning()) {
    Serial.println("RTC NOT running!");
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  }

  // Force time adjustment
  rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));


  while (!Serial) { ; }  // wait for serial port to connect. Needed for Leonardo only

  Serial.println("iero Flower power started");
}

void loop () {
  String debug;

  digitalWrite(ledPin, HIGH);
    
  // Light
  DateTime now = rtc.now();
  debug="Hour : "+String(now.hour())+":"+String(now.minute());
  Serial.println(debug);
  
  if (now.hour() >= LUM_START_HOUR 
    && now.hour() < LUM_STOP_HOUR 
    && now.minute() >= LUM_START_MIN 
    && now.minute() < LUM_STOP_MIN) {
      digitalWrite(lightPin, HIGH);
      debug="Light on";
    } else {
      digitalWrite(lightPin, LOW);
      debug="Light off";
    }
    Serial.println(debug);

  // Moisture
  setSensorPolarity(true);
  delay(MOIST_TIMER);
  int val1 = analogRead(sensorPin);
  //Serial.println("Moist1 : "+String(val1));

  delay(MOIST_TIMER);  
  setSensorPolarity(false);
  delay(MOIST_TIMER);
  // invert the reading
  int val2 = 1023 - analogRead(sensorPin);
  //Serial.println("Moist2 : "+String(val2));
  int avgMoist = (val1 + val2) / 2;
  Serial.println("Moisture : "+String(avgMoist));

  // Water pump  
  if (avgMoist < MOIST_MIN && avgMoist > MOIST_SEC) {
    Serial.println("Watering..");
    digitalWrite(waterPin, HIGH);
    delay(WATER_TIME*1000);
    digitalWrite(waterPin, LOW);
  }

  // DHT22
  /*float h = dht.readHumidity();
  float t = dht.readTemperature();
  debug="Temp : "+String(t)+" degC, Humidity : "+String(h)+"%";
  Serial.println(debug);*/
  
  digitalWrite(ledPin, LOW);
  delay(LOOP);
}
