/*
  Projekt:  BMS Czaple
  Wersja:   20210507
    - optymalizacja kodu




  Wiring
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

// ==========================> INCLUDE <==========================
#include <Arduino.h>
#include <Wire.h>
#include <SPI.h>
#include <VL53L1X.h>
#include <TM1637Display.h>
// ===============================================================

// ===============================================================
#define trigPin 3           //sterowanie czujnika ultradzwiekowego
#define echoPin 4           //sterowanie czujnika ultradzwiekowego
#define CLK 7             //sterowanie wyswietlacza led
#define DIO 5             //sterowanie wyswietlacza led
// ===============================================================

// =====================> DECLARE OBJECTS  <======================
VL53L1X sensor;
TM1637Display display(CLK, DIO);
// ===============================================================



const uint8_t SEG_DONE[] = {
	SEG_B | SEG_C | SEG_D | SEG_E | SEG_G,           // d
	SEG_A | SEG_B | SEG_C | SEG_D | SEG_E | SEG_F,   // O
	SEG_C | SEG_E | SEG_G,                           // n
	SEG_A | SEG_D | SEG_E | SEG_F | SEG_G            // E
	};




void setup() {
  // put your setup code here, to run once:
}

void loop() {
  // put your main code here, to run repeatedly:
}