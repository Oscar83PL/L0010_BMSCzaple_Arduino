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
int counter = 0, DispPageNumber = 0;
unsigned short watchdog = 0;
uint32_t tickTime_Disp = 0;
long duration;
int distance_sonar, distance_laser;

int8_t Disp_A[] = {0x00, 0x00, 0x00, 0x00};

// Allocate the JSON document
// Inside the brackets, 200 is the RAM allocated to this document.
StaticJsonDocument<128> doc;
String output;

//------------------------------------------------------------------------------
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
  sensor.setTimeout(200);
  while (!sensor.init())
  {
    Serial.println(F("Failed to detect and initialize ToF sensor!"));
  }

  // Start LoRa
  LoRa.setTxPower(2);
  while (!LoRa.begin(433E6))
  {
    Serial.println(F("Failed to detect and initialize LoRa!"));
  }
  delay(500);
  LoRa.setTxPower(2);



  // Use long distance mode and allow up to 50000 us (50 ms) for a measurement.
  // You can change these settings to adjust the performance of the sensor, but
  // the minimum timing budget is 20 ms for short distance mode and 33 ms for
  // medium and long distance modes. See the VL53L1X datasheet for more
  // information on range and timing limits.
  sensor.setDistanceMode(VL53L1X::Medium);
  sensor.setMeasurementTimingBudget(200000);

  // Start continuous readings at a rate of one measurement every 50 ms (the
  // inter-measurement period). This period should be at least as long as the
  // timing budget.
  sensor.startContinuous(1000);

  out_Display_A.init();
  out_Display_A.set(BRIGHT_TYPICAL);//BRIGHT_TYPICAL = 2,BRIGHT_DARKEST = 0,BRIGHTEST = 7;

}                                           //--(end setup )---



void loop()
{




  if (tickTime_Disp <= millis()) {
    tickTime_Disp = millis() + 2000;

    switch (DispPageNumber) {
      case 1:
        sensor.read();
        Serial.print("range: ");
        Serial.print(sensor.ranging_data.range_mm);
        Serial.print("\tstatus: ");
        Serial.print(VL53L1X::rangeStatusToString(sensor.ranging_data.range_status));
        Serial.print("\tpeak signal: ");
        Serial.print(sensor.ranging_data.peak_signal_count_rate_MCPS);
        Serial.print("\tambient: ");
        Serial.print(sensor.ranging_data.ambient_count_rate_MCPS);

        distance_laser = sensor.ranging_data.range_mm / 10;
        //  if (sensor.timeoutOccurred()) { distance_laser = 9999; }
        if (sensor.timeoutOccurred()) {
          Serial.print(" TIMEOUT");
        }


        Disp_A[3] = distance_laser % 10;
        Disp_A[2] = (distance_laser / 10) % 10;
        Disp_A[1] = (distance_laser / 100) % 10;
        Disp_A[0] = 12;
        out_Display_A.display(Disp_A);

        break;
      case 2:
        // Assign values in the JSON document
        doc["watchdog"] = watchdog;
        doc["distance_sonar"] = distance_sonar;
        doc["distance_laser"] = distance_laser;

        // Generate the prettified JSON and send it to the Serial port.
        output = "";
        serializeJsonPretty(doc, output);
        Serial.println(output);
            // send packet
            LoRa.beginPacket();
            LoRa.print(output);
            LoRa.endPacket(true);
        break;
      //      case 3:
      //        ;
      //        break;
      default:
        DispPageNumber = 0;

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


        Disp_A[3] = distance_sonar % 10;
        Disp_A[2] = (distance_sonar / 10) % 10;
        Disp_A[1] = (distance_sonar / 100) % 10;
        Disp_A[0] = 10;
        out_Display_A.display(Disp_A);

        break;
    }
    DispPageNumber += 1;
    watchdog += 1;
    if (watchdog > 255) {watchdog = 0;}
  }



}
