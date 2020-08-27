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
#include <SPI.h>
#include <LoRa.h>
#include <VL53L1X.h>      //ToF laserowy czujnik odleglosci 
#include "TM1637.h"       //wyswietlacz 7-segmentowy

// ===============================================================
// ==========================> DEFINE  <==========================
// ===============================================================
#define trigPin 4           //sterowanie czujnika ultradzwiekowego
#define echoPin 3           //sterowanie czujnika ultradzwiekowego
#define CLK_A 7             //sterowanie wyswietlacza led
#define DIO_A 5             //sterowanie wyswietlacza led

// ===============================================================
// =====================> DECLARE OBJECTS  <======================
// ===============================================================
VL53L1X sensor;
TM1637 out_Display_A(CLK_A,DIO_A);


// ===============================================================
// =====================> GLOBAL VARIABLE  <======================
// ===============================================================
int counter = 0;
short watchdog = 0;

long duration;
int distance_sonar, distance_laser, distance_display;

int8_t Disp_A[] = {0x00,0x00,0x00,0x00};

//------------------------------------------------------------------------------
void setup() /****** SETUP: RUNS ONCE ******/
{
  // Define inputs and outputs
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  pinMode(LED_BUILTIN, OUTPUT);
  
  // Begin Serial communication at a baudrate of 9600:
  Serial.begin(9600);
  Serial.println(F("Serial ready"));
  
  Wire.begin();
  Wire.setClock(400000L);
  Serial.println(F("Wire ready"));
  
  // Start ToF sensor
  sensor.setTimeout(500);
  while(!sensor.init())
  {
    Serial.println(F("Failed to detect and initialize ToF sensor!"));
    delay(1000);
    digitalWrite(LED_BUILTIN, HIGH);
    delay(1000);
    digitalWrite(LED_BUILTIN, LOW);
  }

  // Start LoRa
  while(!LoRa.begin(433E6))
  {
    Serial.println(F("Failed to detect and initialize LoRa!"));
    delay(2000);
    digitalWrite(LED_BUILTIN, HIGH);
    delay(2000);
    digitalWrite(LED_BUILTIN, LOW);
  }
  delay(500);

// Use long distance mode and allow up to 50000 us (50 ms) for a measurement.
  // You can change these settings to adjust the performance of the sensor, but
  // the minimum timing budget is 20 ms for short distance mode and 33 ms for
  // medium and long distance modes. See the VL53L1X datasheet for more
  // information on range and timing limits.
  sensor.setDistanceMode(VL53L1X::Short);
  sensor.setMeasurementTimingBudget(5000000);

  // Start continuous readings at a rate of one measurement every 50 ms (the
  // inter-measurement period). This period should be at least as long as the
  // timing budget.
  sensor.startContinuous(500);

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
  distance_sonar = duration*0.034/2;
  
  // Print the distance on the Serial Monitor (Ctrl+Shift+M):
  Serial.print(F("Distance = "));
  Serial.print(distance_sonar);
  Serial.print(F(" cm     | "));
  
  distance_laser = sensor.read() / 10;
  Serial.print(distance_laser);
  if (sensor.timeoutOccurred()) { Serial.print(F(" TIMEOUT")); }
  Serial.print(F(" cm     | "));
  
  Serial.println(watchdog);

  if (watchdog % 2)
  {
    distance_display = distance_sonar; 
  }
  else
  {
    distance_display = distance_laser; 
  }
   
  Disp_A[3] = distance_display % 10;
  Disp_A[2] = (distance_display / 10) % 10;
  Disp_A[1] = (distance_display / 100) % 10;
  Disp_A[0] = (distance_display / 1000) % 10;
  out_Display_A.display(Disp_A);

//
//  // send packet
//  LoRa.beginPacket();
//  LoRa.print("Begin; Watchdog: ");
//  LoRa.print(watchdog);
//  LoRa.print("; L: ");
//  LoRa.print(distance);
//  LoRa.print("; ");
//  LoRa.endPacket();


  delay(1000);
  watchdog += 1;
  if (watchdog > 200)
  {
    watchdog = 0;
  }
}
