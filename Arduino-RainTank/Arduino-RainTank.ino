/*  
    Projekt:  BMS Czaple
    Wersja:   20200719
                      - optymalizacja kodu


*/


// ===============================================================
// ==========================> INCLUDE <==========================
// ===============================================================
#include <Wire.h>
#include "SSD1306Ascii.h"
#include "SSD1306AsciiWire.h"
#include "Adafruit_VL53L0X.h"
#include <SPI.h>
#include <LoRa.h>


// ===============================================================
// ==========================> DEFINE  <==========================
// ===============================================================
#define I2C_ADDRESS 0x3C      // oled SSD1306 address wire (0X3C+SA0 - 0x3C or 0x3D)
#define RST_PIN -1            // oled proper RST_PIN if required.


// ===============================================================
// =====================> DECLARE OBJECTS  <======================
// ===============================================================
SSD1306AsciiWire oled;
Adafruit_VL53L0X lox = Adafruit_VL53L0X();



// ===============================================================
// =====================> GLOBAL VARIABLE  <======================
// ===============================================================
int counter = 0;
short watchdog = 0;

//------------------------------------------------------------------------------
void setup() /****** SETUP: RUNS ONCE ******/
{
  Wire.begin();
  Wire.setClock(400000L);

  #if RST_PIN >= 0
    oled.begin(&Adafruit128x32, I2C_ADDRESS, RST_PIN);
  #else  // RST_PIN <= 0
    oled.begin(&Adafruit128x32, I2C_ADDRESS);
  #endif 

  oled.setFont(Adafruit5x7);
  oled.set1X();

  oled.print(F("LoRa... "));
  if (!LoRa.begin(433E6))
  {
    oled.print(F("failed!"));
    while (1)
      ;
  }
  delay(500);
  oled.println(F(" ok."));

  delay(500);
  oled.print("VL53L0X.. ");
  if (!lox.begin())
  {
    oled.print(F("failed!"));
    while (1)
      ;
  }
  delay(500);
  oled.println(F(" ok."));
  delay(3000);
}                                           //--(end setup )---



void loop()
{

  oled.set1X();

  VL53L0X_RangingMeasurementData_t measure;

  lox.rangingTest(&measure, false); // pass in 'true' to get debug data printout!

  oled.set2X();
  oled.clear();
  oled.print("L:");
  if (measure.RangeStatus != 4)
  { // phase failures have incorrect data
    oled.print(measure.RangeMilliMeter);
  }
  else
  {
    oled.print(">1400");
  }
  oled.println("mm   ");

  oled.set1X();
  oled.print(watchdog);

  // send packet
  LoRa.beginPacket();
  LoRa.print("hello ");
  LoRa.print(watchdog);
  LoRa.print("L: ");
  LoRa.print(measure.RangeMilliMeter);
  LoRa.endPacket();

  delay(1000);
  watchdog += 1;
  if (watchdog > 200)
  {
    watchdog = 0;
  }
}
