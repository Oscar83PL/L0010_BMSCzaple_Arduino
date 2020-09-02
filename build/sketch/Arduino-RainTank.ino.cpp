#include <Arduino.h>
#line 1 "c:\\Users\\Oskar\\Desktop\\Projekty\\20. Arduino\\L0010_BMSCzaple_Arduino\\Arduino-RainTank\\Arduino-RainTank.ino"
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
#include <ArduinoJson.h>

// ===============================================================
// ==========================> DEFINE  <==========================
// ===============================================================
#define trigPin 3           //sterowanie czujnika ultradzwiekowego
#define echoPin 4           //sterowanie czujnika ultradzwiekowego
#define CLK_A 7             //sterowanie wyswietlacza led
#define DIO_A 5             //sterowanie wyswietlacza led

// ===============================================================
// =====================> DECLARE OBJECTS  <======================
// ===============================================================
VL53L1X sensor;
TM1637 out_Display_A(CLK_A, DIO_A);


// ===============================================================
// =====================> GLOBAL VARIABLE  <======================
// ===============================================================
int counter = 0;
short watchdog = 0;

long duration;
int distance_sonar, distance_laser, distance_display;

int8_t Disp_A[] = {0x00, 0x00, 0x00, 0x00};

// Allocate the JSON document
// Inside the brackets, 200 is the RAM allocated to this document.
StaticJsonDocument<128> doc;
String output;

//------------------------------------------------------------------------------
#line 79 "c:\\Users\\Oskar\\Desktop\\Projekty\\20. Arduino\\L0010_BMSCzaple_Arduino\\Arduino-RainTank\\Arduino-RainTank.ino"
void setup();
#line 129 "c:\\Users\\Oskar\\Desktop\\Projekty\\20. Arduino\\L0010_BMSCzaple_Arduino\\Arduino-RainTank\\Arduino-RainTank.ino"
void loop();
#line 79 "c:\\Users\\Oskar\\Desktop\\Projekty\\20. Arduino\\L0010_BMSCzaple_Arduino\\Arduino-RainTank\\Arduino-RainTank.ino"
void setup() /****** SETUP: RUNS ONCE ******/
{
  // Define inputs and outputs
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);

  // Begin Serial communication at a baudrate of 9600:
  Serial.begin(115200);
  Serial.println(F("Serial ready"));

  Wire.begin();
  Wire.setClock(400000L);
  Serial.println(F("Wire ready"));

  // Start ToF sensor
  sensor.setTimeout(500);
  while (!sensor.init())
  {
    Serial.println(F("Failed to detect and initialize ToF sensor!"));
  }

  // Start LoRa
  while (!LoRa.begin(433E6))
  {
    Serial.println(F("Failed to detect and initialize LoRa!"));
  }
  delay(500);



  // Use long distance mode and allow up to 50000 us (50 ms) for a measurement.
  // You can change these settings to adjust the performance of the sensor, but
  // the minimum timing budget is 20 ms for short distance mode and 33 ms for
  // medium and long distance modes. See the VL53L1X datasheet for more
  // information on range and timing limits.
  sensor.setDistanceMode(VL53L1X::Long);
  sensor.setMeasurementTimingBudget(50000);

  // Start continuous readings at a rate of one measurement every 50 ms (the
  // inter-measurement period). This period should be at least as long as the
  // timing budget.
  sensor.startContinuous(50);

  out_Display_A.init();
  out_Display_A.set(BRIGHT_TYPICAL);//BRIGHT_TYPICAL = 2,BRIGHT_DARKEST = 0,BRIGHTEST = 7;

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
  distance_sonar = duration * 0.034 / 2;

  distance_laser = sensor.read() / 10;
  //  if (sensor.timeoutOccurred()) { distance_laser = 9999; }
  if (sensor.timeoutOccurred()) {
    Serial.print(" TIMEOUT");
  }

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

  watchdog += 1;
  //  if (watchdog > 200)
  //  {
  //    watchdog = 0;
  //  }

  // Assign values in the JSON document
  doc["watchdog"] = watchdog;
  doc["distance_sonar"] = distance_sonar;
  doc["distance_laser"] = distance_laser;

  // Generate the prettified JSON and send it to the Serial port.
  output = "";
  serializeJsonPretty(doc, output);
  Serial.println(output);
  //serializeJsonPretty(doc, LoRa);

  // send packet
  LoRa.beginPacket();
  LoRa.print(output);
  LoRa.endPacket();

  //delay(1000);
}

