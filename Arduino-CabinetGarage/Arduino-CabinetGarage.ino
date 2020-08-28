/*  

*/

// ===============================================================
// ==========================> INCLUDE <==========================
// ===============================================================
#include <SPI.h>
#include <LoRa.h>
#include <ArduinoJson.h>


// ===============================================================
// ==========================> DEFINE  <==========================
// ===============================================================


// ===============================================================
// =====================> DECLARE OBJECTS  <======================
// ===============================================================



// ===============================================================
// =====================> GLOBAL VARIABLE  <======================
// ===============================================================
short watchdog = 0;
int distance_sonar, distance_laser;
String incoming;
// Allocate the JSON document
// Inside the brackets, 200 is the RAM allocated to this document.
StaticJsonDocument<128> doc;


// ===============================================================
// ====================> ETHERNET SETTINGS  <=====================
// ===============================================================

void setup() /****** SETUP: RUNS ONCE ******/
{
  Serial.begin(9600);
  while (!Serial);

  Serial.println("LoRa Receiver");

  if (!LoRa.begin(433E6)) {
    Serial.println("Starting LoRa failed!");
    while (1);
  }

  
} /****** SETUP: RUNS ONCE (end setup ) ******/

void loop() /****** LOOP: RUNS CONSTANTLY ******/
{
  // try to parse packet
  int packetSize = LoRa.parsePacket();
  if (packetSize) {
    // received a packet
    Serial.print("Received packet '");

    incoming = "";
    // read packet
    while (LoRa.available()) {
      //Serial.print((char)LoRa.read());
      incoming += (char)LoRa.read();
    }
    
    Serial.println(incoming);
    // print RSSI of packet
    Serial.print("' with RSSI ");
    Serial.println(LoRa.packetRssi());
  }

  // Deserialize the JSON document
  DeserializationError error = deserializeJson(doc, incoming);

  // Test if parsing succeeds.
  if (error) {
    Serial.print(F("deserializeJson() failed: "));
    Serial.println(error.c_str());
    return;
  }

  // Fetch values.
  // Most of the time, you can rely on the implicit casts.
  // In other case, you can do doc["time"].as<long>();
  watchdog = doc["watchdog"];
  distance_sonar = doc["distance_sonar"];
  distance_laser = doc["distance_laser"];


  
} /****** LOOP: RUNS CONSTANTLY (end main loop  ******/
