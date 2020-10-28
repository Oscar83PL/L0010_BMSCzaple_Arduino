/*  

*/

// ===============================================================
// ==========================> INCLUDE <==========================
// ===============================================================
#include <DallasTemperature.h>
#include <Adafruit_BMP085.h>
#include <Adafruit_BME280.h>
#include <SPI.h>
#include <LoRa.h>
#include <ArduinoJson.h>
#include "SSD1306Ascii.h"
#include "SSD1306AsciiWire.h"
#include "Platform.h"
#include "Settimino.h"
#include "TempSensors.h"

// ===============================================================
// ==========================> DEFINE  <==========================
// ===============================================================
#define OLED_I2C_ADDRESS 0x3C    // 0X3C+SA0 - 0x3C or 0x3D
#define OLED_RST_PIN -1          // Define proper RST_PIN if required.
#define OLED_RTN_CHECK 1        // Try values of zero or one for RTN_CHECK.

#define LORA_SS 4
#define LORA_RST 3
#define LORA_DIO0 2

#define ONE_WIRE_BUS_PIN_LINE_A 6
#define ONE_WIRE_BUS_PIN_LINE_B 7
#define ONE_WIRE_BUS_PIN_LINE_C 8
#define ONE_WIRE_BUS_PIN_LINE_D 9

#define TEMPERATURE_PRECISION 12
#define ENABLE_SEARCH_ADDRESS       // Comment it out if you don't want to use it.
#define ENABLE_READ_BACK_RESOLUTION // Comment it out if you don't want to use it.

#define SEALEVELPRESSURE_HPA (1013.25)

#define GSM_Serial Serial1


// ===============================================================
// =====================> DECLARE OBJECTS  <======================
// ===============================================================
SSD1306AsciiWire oled;

OneWire oneWireLines[] = {ONE_WIRE_BUS_PIN_LINE_A,
                          ONE_WIRE_BUS_PIN_LINE_B,
                          ONE_WIRE_BUS_PIN_LINE_C,
                          ONE_WIRE_BUS_PIN_LINE_D,
                          };
const int oneWireCount = sizeof(oneWireLines) / sizeof(OneWire);
DallasTemperature sensor[oneWireCount];

Adafruit_BMP085 bmp;
Adafruit_BME280 bme;


// ===============================================================
// =====================> GLOBAL VARIABLE  <======================
// ===============================================================
short watchdog = 0;
int distance_sonar, distance_laser;
int OledPageNumber = 0;
int LoRaRSSI = 0;
String incoming;
uint32_t tickTime_OledPage = 0;
uint32_t tickTime_CommPLC = 0;

// Allocate the JSON document
// Inside the brackets, 200 is the RAM allocated to this document.
StaticJsonDocument<128> doc;
// Use two strings to avoid modifying string being displayed.

// ===============================================================
// ====================> ETHERNET SETTINGS  <=====================
// ===============================================================
byte mac[] = {0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xEE};
//IPAddress ipArduino(192, 168, 8, 216); // Local Address
//IPAddress gateway(192, 168, 8, 1);     // Local Gateway
//IPAddress subnet(255, 255, 255, 0);    // Local Subnet
//IPAddress ipPLC(192, 168, 8, 210);     // Remote PLC Address

IPAddress ipArduino(192,168,51,200);                  // Local Address
IPAddress ipPLC(192,168,51,199);                      // PLC Address
IPAddress gateway(192, 168, 51, 100);
IPAddress subnet(255, 255, 255, 0);

/*-----( Declare Variables )-----*/
int DBNum = 251; // This DB must be present in your PLC
byte Buffer[1024];
unsigned long Elapsed; // To calc the execution time
bool PLC_ConectionEstablished = false;

// S7Client will create an EthernetClient as TCP Client
S7Client Client;


void setup() /****** SETUP: RUNS ONCE ******/
{
  Wire.begin();
  Wire.setClock(400000L);
  
  oled.begin(&Adafruit128x32, OLED_I2C_ADDRESS);
  //oled.setFont(Callibri11);
  oled.setFont(Adafruit5x7);
  oled.clear();
  oled.set2X();
  
  oled.println("Display OLED ... ");
  delay(250);
  oled.println("OK");
  delay(750);
  
  oled.clear();
  oled.println("Serial ... ");
  Serial.begin(9600);
  while (!Serial);
  delay(250);
  oled.println("OK");
  delay(750);
  
  
  Serial.println("LoRa Receiver");


  pinMode(LORA_SS, OUTPUT);
  digitalWrite(LORA_SS, HIGH);
  LoRa.setPins(LORA_SS, LORA_RST, LORA_DIO0);
  oled.clear();
  oled.println("LoRa module ...  ");
  if (!LoRa.begin(433E6)) {
    Serial.println("Starting LoRa failed!");
    while (1);
  }
  delay(250);
  oled.println("OK");
  delay(750);
  
  //--------------------------------Wired Ethernet Shield Initialization
  //Start the Ethernet Library
  EthernetInit(mac, ipArduino);

  //Setup Time, someone said me to leave 2000 because some
  //rubbish compatible boards are a bit deaf.
  delay(2000);
  Serial.println("");
  Serial.println("Cable connected");
  Serial.print("Local IP address : ");
  Serial.println(Ethernet.localIP());
  //oled.clear();
  oled.clear();
  oled.println("LAN ...   ");
  delay(250);
  oled.println(Ethernet.localIP());
  delay(750);
  
// Initialize the Temperature measurement library
  Serial.println("");
  Serial.print("Initializing DS18b20 Sensors... ");
  Serial.println(DALLASTEMPLIBVERSION);
  Serial.println("====== Dallas Temperature Multiple Bus Control ======");
  Serial.print("=========== Ready with ");
  Serial.print(oneWireCount);
  Serial.println(" lines Sensors ===============");

  delay(1000);
  #ifdef ENABLE_SEARCH_ADDRESS // Get the number of devices on the 1-Wire bus.
    for (int i = 0; i < oneWireCount; i++)
    {
      ;
      sensor[i].setOneWire(&oneWireLines[i]);
      sensor[i].begin();
      //getDeviceAddress(i);
      //if (sensor[i].getAddress(deviceAddress, 0))
      //sensor[i].setResolution(deviceAddress, 12);
    }
  #endif

    // set the resolution (Can be 9 to 12 bits .. lower is faster)
    for (int i = 0; i < NUM_SENSORS_LINE_A; i++)
    {
      sensor[0].setResolution(Sensor_Address_Line_A[i], TEMPERATURE_PRECISION);
      #ifdef ENABLE_READ_BACK_RESOLUTION
          Serial.print(F("resolution readback line A, sensor no. "));
          Serial.print(i);
          Serial.print(F(" = "));
          Serial.print(sensor[0].getResolution(Sensor_Address_Line_A[i]), DEC);
          Serial.println();
      #endif
    }
    for (int i = 0; i < NUM_SENSORS_LINE_B; i++)
    {
      sensor[1].setResolution(Sensor_Address_Line_B[i], TEMPERATURE_PRECISION);
      #ifdef ENABLE_READ_BACK_RESOLUTION
          Serial.print(F("resolution readback line B, sensor no. "));
          Serial.print(i);
          Serial.print(F("= "));
          Serial.print(sensor[1].getResolution(Sensor_Address_Line_B[i]), DEC);
          Serial.println();
      #endif
    }
    for (int i = 0; i < NUM_SENSORS_LINE_C; i++)
    {
      sensor[1].setResolution(Sensor_Address_Line_B[i], TEMPERATURE_PRECISION);
      #ifdef ENABLE_READ_BACK_RESOLUTION
          Serial.print(F("resolution readback line C, sensor no. "));
          Serial.print(i);
          Serial.print(F("= "));
          Serial.print(sensor[1].getResolution(Sensor_Address_Line_B[i]), DEC);
          Serial.println();
      #endif
    }
    for (int i = 0; i < NUM_SENSORS_LINE_D; i++)
    {
      sensor[1].setResolution(Sensor_Address_Line_B[i], TEMPERATURE_PRECISION);
      #ifdef ENABLE_READ_BACK_RESOLUTION
          Serial.print(F("resolution readback line D, sensor no. "));
          Serial.print(i);
          Serial.print(F("= "));
          Serial.print(sensor[1].getResolution(Sensor_Address_Line_B[i]), DEC);
          Serial.println();
      #endif
    }

  oled.clear();
  oled.println("Setup finished :D");
  
  delay(3000);
  oled.clear();
} /****** SETUP: RUNS ONCE (end setup ) ******/

void loop() /****** LOOP: RUNS CONSTANTLY ******/
{
  // try to parse packet
  int packetSize = LoRa.parsePacket();
  if (packetSize) {
    // received a packet
    Serial.print("Received packet ' - ");
    Serial.println(packetSize);

    incoming = "";
    // read packet
    while (LoRa.available()) {
      //Serial.print((char)LoRa.read());
      incoming += (char)LoRa.read();
    }
    
    Serial.println(incoming);
    // print RSSI of packet
    Serial.print("' with RSSI ");
    LoRaRSSI = LoRa.packetRssi();
    Serial.println(LoRaRSSI);
  
    // Deserialize the JSON document
    DeserializationError error = deserializeJson(doc, incoming);
    // Test if parsing succeeds.
    if (error) {
      Serial.print(F("Json() failed: "));
      Serial.println(error.c_str());
      oled.clear();
      oled.println(F("Json() failed: "));
      oled.println(error.c_str());
      return;
     }  
      // Fetch values.
      // Most of the time, you can rely on the implicit casts.
      // In other case, you can do doc["time"].as<long>();
      watchdog = doc["watchdog"];
      distance_sonar = doc["distance_sonar"];
      distance_laser = doc["distance_laser"];


  }

  Serial.println();

  if (sensor[0].getDeviceCount()) {sensor[0].requestTemperatures();}
  if (sensor[1].getDeviceCount()) {sensor[1].requestTemperatures();}
  if (sensor[2].getDeviceCount()) {sensor[2].requestTemperatures();}
  if (sensor[3].getDeviceCount()) {sensor[3].requestTemperatures();}
  
  
  Serial.println("=== DS18b20 temperature line A ===");
  for (int i = 0; i < NUM_SENSORS_LINE_A; i++)
  {
    Serial.print("DS18b20 no. ");
    Serial.print(i);
    Serial.print(" temperature is:   ");
    printTemperature(0, Sensor_Address_Line_A[i]);
    Serial.println();
  }

  Serial.println("=== DS18b20 temperature line B ===");
  for (int i = 0; i < NUM_SENSORS_LINE_B; i++)
  {
    Serial.print("DS18b20 no. ");
    Serial.print(i);
    Serial.print(" temperature is:   ");
    printTemperature(1, Sensor_Address_Line_B[i]);
    Serial.println();
  }

  Serial.println("=== DS18b20 temperature line C ===");
  for (int i = 0; i < NUM_SENSORS_LINE_C; i++)
  {
    Serial.print("DS18b20 no. ");
    Serial.print(i);
    Serial.print(" temperature is:   ");
    printTemperature(0, Sensor_Address_Line_C[i]);
    Serial.println();
  }

  Serial.println("=== DS18b20 temperature line D ===");
  for (int i = 0; i < NUM_SENSORS_LINE_D; i++)
  {
    Serial.print("DS18b20 no. ");
    Serial.print(i);
    Serial.print(" temperature is:   ");
    printTemperature(1, Sensor_Address_Line_D[i]);
    Serial.println();
  }

  if (tickTime_OledPage <= millis()) {
    tickTime_OledPage = millis() + 5000;

    
    oled.clear();
    switch (OledPageNumber) {
      case 1:    
        oled.print(F("Sonar: "));
        oled.print(distance_sonar);
        oled.println(F(" cm"));
        oled.print(F("Laser: "));
        oled.print(distance_laser);
        oled.println(F(" cm")); ;
        break;
      case 2:    
        ;
        break;
      case 3:    
        oled.print(F("RSSI: "));
        oled.println(LoRaRSSI);
        break;
      case 4:    
        oled.print(F("PLC: "));
        oled.println(PLC_ConectionEstablished);
        break;
      default:
        OledPageNumber = 0;
        oled.println(F("Watchdog tank: "));
        oled.println(watchdog);
        break;
    }
    OledPageNumber += 1;

  }


  if (tickTime_CommPLC <= millis()) {
    tickTime_CommPLC = millis() + 2000;

  int Size, Result;
  void *Target;

  Size = 64;
  Target = &Buffer; // Uses a larger buffer

  if (Client.Connected)
  {
    Serial.println(" client connected");
  }

  if (Connect())
  {
    Serial.println("connectinging ok");
  }

  if (Client.Connected and Connect())
  {
    
    // Read PLC DBxxx
    Serial.print("Reading ");
    Serial.print(Size);
    Serial.print(" bytes from DB");
    Serial.println(DBNum);
    MarkTime();
    Result = Client.ReadArea(S7AreaDB, // We are requesting DB access
                             DBNum,    // DB Number
                             0,        // Start from byte N.0
                             Size,     // We need "Size" bytes
                             Target);  // Put them into our target (Buffer or PDU)
    if (Result == 0)
    {
      ShowTime();
      Dump(Target, Size);
    }
    else
      CheckError(Result);  
    
    S7.SetIntAt(Target, 10, sensor[0].getTempC(Sensor_Address_Line_A[0]) * 100);
    S7.SetIntAt(Target, 12, sensor[0].getTempC(Sensor_Address_Line_A[1]) * 100);
    S7.SetIntAt(Target, 14, sensor[0].getTempC(Sensor_Address_Line_A[2]) * 100);
    S7.SetIntAt(Target, 16, sensor[0].getTempC(Sensor_Address_Line_A[3]) * 100);
    S7.SetIntAt(Target, 18, sensor[0].getTempC(Sensor_Address_Line_A[4]) * 100);
    S7.SetIntAt(Target, 20, sensor[0].getTempC(Sensor_Address_Line_A[5]) * 100);
    S7.SetIntAt(Target, 22, sensor[0].getTempC(Sensor_Address_Line_A[6]) * 100);
    S7.SetIntAt(Target, 24, sensor[0].getTempC(Sensor_Address_Line_A[7]) * 100);
    S7.SetIntAt(Target, 26, sensor[0].getTempC(Sensor_Address_Line_A[8]) * 100);

    S7.SetIntAt(Target, 28, sensor[1].getTempC(Sensor_Address_Line_B[0]) * 100);


    S7.SetIntAt(Target, 0, watchdog);
    S7.SetIntAt(Target, 50, distance_sonar);
    S7.SetIntAt(Target, 52, distance_laser);
    
MarkTime();
    Result = Client.WriteArea(S7AreaDB, // We are requesting DB access
                              DBNum,    // DB Number
                              0,        // Start from byte N.0
                              Size,     // We need "Size" bytes
                              Target);  // Put them into our target (Buffer or PDU)
    if (Result == 0)
    {
      ShowTime();
      Dump(Target, Size);
      PLC_ConectionEstablished = true;
    }
    else
    {
      CheckError(Result);
      PLC_ConectionEstablished = false;
    }
  }
  else //((!Client.Connected) or (!Connect()))
  {
    //delay(500);
    Serial.println("PLC not connected");
    
  }

  }




  
} /****** LOOP: RUNS CONSTANTLY (end main loop  ******/


//----------------------------------------------------------------------
// Connects to the PLC
//----------------------------------------------------------------------
bool Connect()
{
  int Result = Client.ConnectTo(ipPLC,
                                0,  // Rack (see the doc.)
                                1); // Slot (see the doc.)
  Serial.print("Connecting to ");
  Serial.println(ipPLC);
  if (Result == 0)
  {
    Serial.print("Connected ! PDU Length = ");
    Serial.println(Client.GetPDULength());
  }
  else
    Serial.println("Connection error");
  return Result == 0;
}
//----------------------------------------------------------------------
// Dumps a buffer (a very rough routine)
//----------------------------------------------------------------------
void Dump(void *Buffer, int Length)
{
  int i, cnt = 0;
  pbyte buf;

  if (Buffer != NULL)
    buf = pbyte(Buffer);
  else
    buf = pbyte(&PDU.DATA[0]);

  Serial.print("[ Dumping ");
  Serial.print(Length);
  Serial.println(" bytes ]===========================");
  for (i = 0; i < Length; i++)
  {
    cnt++;
    if (buf[i] < 0x10)
      Serial.print("0");
    Serial.print(buf[i], HEX);
    Serial.print(" ");
    if (cnt == 16)
    {
      cnt = 0;
      Serial.println();
    }
  }
  Serial.println("===============================================");
}
//----------------------------------------------------------------------
// Prints the Error number
//----------------------------------------------------------------------
void CheckError(int ErrNo)
{
  Serial.print("Error No. 0x");
  Serial.println(ErrNo, HEX);

  // Checks if it's a Severe Error => we need to disconnect
  if (ErrNo & 0x00FF)
  {
    Serial.println("SEVERE ERROR, disconnecting.");
    Client.Disconnect();
  }
}
//----------------------------------------------------------------------
// Profiling routines
//----------------------------------------------------------------------
void MarkTime()
{
  Elapsed = millis();
}
//----------------------------------------------------------------------
void ShowTime()
{
  // Calcs the time
  Elapsed = millis() - Elapsed;
  Serial.print("Job time (ms) : ");
  Serial.println(Elapsed);
}

/*-----( Declare User-written Functions )-----*/
void printTemperature(int OneWireLine, DeviceAddress deviceAddress)
{

  float tempC = sensor[OneWireLine].getTempC(deviceAddress);

  if (tempC == -127.00)
  {
    Serial.print("Error getting temperature  ");
  }
  else
  {
    Serial.print("C: ");
    Serial.print(tempC);
  }
}

void getDeviceAddress(int OneWireLine)
{

  int n = sensor[OneWireLine].getDeviceCount();
  DeviceAddress deviceAddress; // temporary sensor address.

  Serial.print("Number of DS18B20 found on bus [");
  Serial.print(OneWireLine);
  Serial.print("] = ");
  Serial.println(n);

  for (int i = 0; i < n; i++) // this is the index for the DallasTemperature library
  {
    sensor[i].getAddress(deviceAddress, i);
    Serial.print(F("Address: "));
    for (int j = 0; j < 8; j++)
    {
      Serial.print(F("0x"));
      if (deviceAddress[j] < 16)
        Serial.print(F("0"));
      Serial.print(deviceAddress[j], HEX);
      if (j != 7)
        Serial.print(F(", "));
    }
    // report parasite power requirements
    Serial.print(" / Parasite power is: ");
    if (sensor[i].isParasitePowerMode())
      Serial.println("ON");
    else
      Serial.println("OFF");
    Serial.println();
  }

  Serial.println();
  // a check to make sure that what we read is correct.
  if (OneWire::crc8(deviceAddress, 7) != deviceAddress[7])
  {
    Serial.print("CRC is not valid!\n");
    return;
  }

  oneWireLines[OneWireLine].reset_search();
  return;
}