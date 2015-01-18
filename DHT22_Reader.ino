#include <dht.h>

#define LOG_INTERVAL  1000 // mills between entries (reduce to take more/faster data)
// The analog pins that connect to the sensors
#define dht22Pin 2           // analog 1

dht DHT; // define DHT22 object

void setup(void) {
  Serial.begin(9600);
  Serial.println();
  Serial.println("Arduino started");
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

}
