/*  
    Projekt:  BMS Czaple
    Wersja:   20200719
                      - optymalizacja kodu




    Wiring
          LoRa.GND  < = > GND
          LoRa.3.3  < = > 3.3V
          LoRa.RST  < = > UNO.D9
          LoRa.DIO  < = > UNO.D2
          LoRa.NSS  < = > UNO.D10
          LoRa.MOSI < = > UNO.D11
          LoRa.MISO < = > UNO.D12
          LoRa.SCK  < = > UNO.D13

          oled.GND  < = > GND
          oled.VCC  < = > 3.3V
          oled.SCL  < = > UNO.A5 (SCL)
          oled.SDA  < = > UNO.A4 (SDA)

          VL53L1X.GND  < = > GND
          VL53L1X.VCC  < = > 3.3V
          VL53L1X.SCL  < = > UNO.A5 (SCL)
          VL53L1X.SDA  < = > UNO.A4 (SDA)

          JSN-SR04T.GND  < = > GND
          JSN-SR04T.VCC  < = > 5V
          JSN-SR04T.Echo < = > UNO.D3 (SCL)
          JSN-SR04T.Trg  < = > UNO.D4 (SDA) 
          
*/


// ===============================================================
// ==========================> INCLUDE <==========================
// ===============================================================
#include <Wire.h>
#include "SSD1306Ascii.h"
#include "SSD1306AsciiWire.h"
#include <SPI.h>
#include <LoRa.h>
#include <VL53L1X.h>

#include "TM1637.h"



// ===============================================================
// ==========================> DEFINE  <==========================
// ===============================================================
#define I2C_ADDRESS 0x3C      // oled SSD1306 address wire (0X3C+SA0 - 0x3C or 0x3D)
#define RST_PIN -1            // oled proper RST_PIN if required.

#define trigPin 4
#define echoPin 3
#define CLK_A 7
#define DIO_A 5


// ===============================================================
// =====================> DECLARE OBJECTS  <======================
// ===============================================================
SSD1306AsciiWire oled;
VL53L1X sensor;
TM1637 out_Display_A(CLK_A,DIO_A);


// ===============================================================
// =====================> GLOBAL VARIABLE  <======================
// ===============================================================
int counter = 0;
short watchdog = 0;

long duration;
int distance;

int8_t Disp_A[] = {0x00,0x00,0x00,0x00};
int8_t Disp_B[] = {0x00,0x00,0x00,0x00};

//------------------------------------------------------------------------------
void setup() /****** SETUP: RUNS ONCE ******/
{
  // Define inputs and outputs
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  
  // Begin Serial communication at a baudrate of 9600:
  Serial.begin(9600);
  
//  
//  Wire.begin();
//  Wire.setClock(400000L);
//
//  sensor.setTimeout(500);
//  if (!sensor.init())
//  {
//    Serial.println("Failed to detect and initialize sensor!");
//    while (1);
//  }
//
//  #if RST_PIN >= 0
//    oled.begin(&Adafruit128x32, I2C_ADDRESS, RST_PIN);
//  #else // RST_PIN >= 0
//    oled.begin(&Adafruit128x32, I2C_ADDRESS);
//  #endif // RST_PIN >= 0

//  oled.setFont(Adafruit5x7);
//  oled.set1X();
//
//  oled.print(F("LoRa... "));
//  if (!LoRa.begin(433E6))
//  {
//    oled.print(F("failed!"));
//    while (1)
//      ;
//  }
//  delay(500);
//  oled.println(F(" ok."));

// Use long distance mode and allow up to 50000 us (50 ms) for a measurement.
  // You can change these settings to adjust the performance of the sensor, but
  // the minimum timing budget is 20 ms for short distance mode and 33 ms for
  // medium and long distance modes. See the VL53L1X datasheet for more
  // information on range and timing limits.
//  sensor.setDistanceMode(VL53L1X::Long);
//  sensor.setMeasurementTimingBudget(50000);

  // Start continuous readings at a rate of one measurement every 50 ms (the
  // inter-measurement period). This period should be at least as long as the
  // timing budget.
//  sensor.startContinuous(500);

  out_Display_A.init();
  out_Display_A.set(BRIGHT_TYPICAL);//BRIGHT_TYPICAL = 2,BRIGHT_DARKEST = 0,BRIGHTEST = 7;

  delay(3000);
}                                           //--(end setup )---



void loop()
{
  // Clear the trigPin by setting it LOW:
  digitalWrite(trigPin, LOW);
  
  delayMicroseconds(5);
 // Trigger the sensor by setting the trigPin high for 10 microseconds:
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  
  // Read the echoPin. pulseIn() returns the duration (length of the pulse) in microseconds:
  duration = pulseIn(echoPin, HIGH);
  
  // Calculate the distance:
  distance = duration*0.034/2;
  
  // Print the distance on the Serial Monitor (Ctrl+Shift+M):
  Serial.print("Distance = ");
  Serial.print(distance);
  Serial.println(" cm");
  
//  
//  
//  
//  //oled.set2X();
//  oled.clear();
//  oled.print("Le:");
//  oled.print(distance);
//  oled.println("mm   ");
//
//    Serial.print(sensor.read());
//  if (sensor.timeoutOccurred()) { Serial.print(" TIMEOUT"); }
//
//  Serial.println();
//
//  oled.print("Lx:");
//  oled.print(sensor.read());
//  oled.println("mm   ");
//
//
//  
//  //oled.set1X();
//  oled.print(watchdog);
//
//  // send packet
//  LoRa.beginPacket();
//  LoRa.print("Begin; Watchdog: ");
//  LoRa.print(watchdog);
//  LoRa.print("; L: ");
//  LoRa.print(distance);
//  LoRa.print("; ");
//  LoRa.endPacket();

    Disp_A[3] = distance % 10;
    Disp_A[2] = (distance / 10) % 10;
    Disp_A[1] = (distance / 100) % 10;
    Disp_A[0] = (distance / 1000) % 10;
    out_Display_A.display(Disp_A);

  delay(1000);
  watchdog += 1;
  if (watchdog > 200)
  {
    watchdog = 0;
  }
}
