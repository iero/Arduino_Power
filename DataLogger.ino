#include <dht.h>
#include <SD.h>
#include <Wire.h>
#include "RTClib.h"

#define LOG_INTERVAL  5000 // mills between entries (reduce to take more/faster data)
#define SYNC_INTERVAL 5000 // mills between calls to flush() - to write data to the card
uint32_t syncTime = 0; // time of last sync()

#define ECHO_TO_SERIAL   1 // echo data to serial port
#define WAIT_TO_START    0 // Wait for serial input in setup()

// The analog pins that connect to the sensors
#define dht22Pin 2           // analog 1

// for the data logging shield, we use digital pin 10 for the SD cs line
const int chipSelect = 10;

RTC_DS1307 rtc; // define the Real Time Clock object
dht DHT; // define DHT22 object

const int redLEDpin=2;
const int greenLEDpin=3;

// the logging file
File logfile;

void error(char *str)
{
  Serial.print("error: ");
  Serial.println(str);
  
  while(1);
}

void setup(void) {
  Serial.begin(9600);
  Serial.println();
  Serial.println("Arduino started");
  
  Serial.print("RTC");
  Wire.begin();
  rtc.begin();
  if (! rtc.isrunning()) {
    Serial.println("RTC is NOT running!");
    // following line sets the RTC to the date & time this sketch was compiled
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
   Serial.println(" started");
  }
  
  // initialize the SD card
  Serial.print("Initializing SD card...");
  // make sure that the default chip select pin is set to
  // output, even if you don't use it:
  pinMode(10, OUTPUT);
  
  // see if the card is present and can be initialized:
  if (!SD.begin(10, 11, 12, 13)) {
    error("Card failed, or not present");
  }
  Serial.println("card initialized.");
  
  // create a new file
  char filename[] = "LOGGER00.CSV";
  for (uint8_t i = 0; i < 100; i++) {
    filename[6] = i/10 + '0';
    filename[7] = i%10 + '0';
    if (! SD.exists(filename)) {
      // only open a new file if it doesn't exist
      logfile = SD.open(filename, FILE_WRITE); 
      break;  // leave the loop!
    }
  }
  
  if (! logfile) {
    error("couldnt create file");
  }
 
}

void loop(void)
{

  // delay for the amount of time we want between readings
  delay((LOG_INTERVAL -1) - (millis() % LOG_INTERVAL));
  
  // fetch the time
  DateTime now = rtc.now();

  // log time
  logfile.print(now.year(), DEC);
  logfile.print("/");
  logfile.print(now.month(), DEC);
  logfile.print("/");
  logfile.print(now.day(), DEC);
  logfile.print(" ");
  logfile.print(now.hour(), DEC);
  logfile.print(":");
  logfile.print(now.minute(), DEC);
  logfile.print(":");
  logfile.print(now.second(), DEC);

  // Read DHT22 sensor

  uint32_t start = micros(); 
  int chk = DHT.read22(dht22Pin);
  uint32_t stop = micros();
    
  logfile.print(", ");    
  logfile.print(DHT.temperature);
  logfile.print(", ");    
  logfile.print(DHT.humidity);

  Serial.print("Temperature : ");    
  Serial.println(DHT.temperature);
  Serial.print("Humidite : ");    
  Serial.println(DHT.humidity);

  logfile.println();

  // Now we write data to disk! Don't sync too often - requires 2048 bytes of I/O to SD card
  // which uses a bunch of power and takes time
  if ((millis() - syncTime) < SYNC_INTERVAL) return;
  syncTime = millis();
  
  // blink LED to show we are syncing data to the card & updating FAT!
  digitalWrite(redLEDpin, HIGH);
  logfile.flush();
  digitalWrite(redLEDpin, LOW);
  
}
