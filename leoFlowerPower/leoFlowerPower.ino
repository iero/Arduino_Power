// Automated water system

// For RTC clock reading
#include "RTClib.h"*/

//#define LOOP 5000 // debug
#define LOOP 300000  // ms bw two actions.
#define WATER_TIME 1000  // ms to pump

#define LUM_START_HOUR 8
#define LUM_STOP_HOUR 19

#define MOIST_MIN 825 // Min Moisture to start pump
#define MOIST_SEC 500 // To detect if moist sensor is out
#define MOIST_TIMER 5000 // Waiting time to power moisture

#define lightPin 7  // Light switch
#define waterPin 4  // Water Pump

// Moisture
#define voltageFlipPin1 8
#define voltageFlipPin2 9
#define sensorPin A0

boolean forcedWater = false;
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
  forcedWater=false;
  rtc.begin();

  pinMode(voltageFlipPin1, OUTPUT);
  pinMode(voltageFlipPin2, OUTPUT);
  pinMode(sensorPin, INPUT);  
    
  pinMode(lightPin, OUTPUT);
  pinMode(waterPin, OUTPUT);
    
  digitalWrite(lightPin, LOW);
  digitalWrite(waterPin, LOW);

  if (! rtc.begin()) {
    Serial.println("Couldn't find RTC");
    while (1);
  }
   
  if (! rtc.isrunning()) {
    Serial.println("RTC NOT running!");
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  }

  // Force time adjustment
  //rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));

  while (!Serial) { ; }  // wait for serial port to connect. Needed for Leonardo only
  Serial.println("iero Flower power started");
}

void loop () {
  String debug;
    
  // Light
  DateTime now = rtc.now();
  debug=String(now.hour())+":"+String(now.minute());
  
  if (now.hour() >= LUM_START_HOUR 
    && now.hour() < LUM_STOP_HOUR ) {
      digitalWrite(lightPin, HIGH);
      debug+=" ;  Light on ;";
    } else {
      digitalWrite(lightPin, LOW);
      debug+=" ;  Light off ;";
    }

  // Moisture 
  setSensorPolarity(true);
  delay(MOIST_TIMER);
  int val1 = analogRead(sensorPin);
  delay(MOIST_TIMER);  
  setSensorPolarity(false);
  delay(MOIST_TIMER);
  int val2 = 1023 - analogRead(sensorPin);   // invert the reading

  int avgMoist = (val1 + val2) / 2;
  debug+="  Moisture : "+String(avgMoist);
  digitalWrite(voltageFlipPin2, LOW);

  // Water pump  
  if (avgMoist < MOIST_MIN && avgMoist > MOIST_SEC) {
    debug+="  Watering..";
    digitalWrite(waterPin, HIGH);
    delay(WATER_TIME);
    digitalWrite(waterPin, LOW);
  }

  // Reset forced state
  if (avgMoist > MOIST_SEC) forcedWater=false ;

  // Forced state
  if (avgMoist == 0 && !forcedWater) {
    forcedWater = true ;
    debug+="  Force watering..";
    digitalWrite(waterPin, HIGH);
    delay(WATER_TIME);
    digitalWrite(waterPin, LOW);
  }

  Serial.println(debug);
  delay(LOOP);
}
