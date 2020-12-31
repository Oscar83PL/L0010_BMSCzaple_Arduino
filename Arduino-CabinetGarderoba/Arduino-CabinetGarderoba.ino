/*  
    Projekt:  BMS Czaple
    Wersja:   20190207 Optymalizacja kodu
    Wersja:   20190304 Test
    Wersja:   20190824 Dwie linie 1Wire, Upgrade Settimino lib to v2.0
    Wersja:   20200425  Configuracja pod VSCode


*/


// ===============================================================
// ==========================> INCLUDE <==========================
// ===============================================================
#include <DallasTemperature.h>
#include "Platform.h"
#include "Settimino.h"
#include "TempSensors.h"

// ===============================================================
// ==========================> DEFINE  <==========================
// ===============================================================
#define ONE_WIRE_BUS_PIN_LINE_A 6
#define ONE_WIRE_BUS_PIN_LINE_B 7

#define TEMPERATURE_PRECISION 12
#define ENABLE_SEARCH_ADDRESS       // Comment it out if you don't want to use it.
#define ENABLE_READ_BACK_RESOLUTION // Comment it out if you don't want to use it.

#define SEALEVELPRESSURE_HPA (1013.25)

// ===============================================================
// =====================> DECLARE OBJECTS  <======================
// ===============================================================
OneWire oneWireLines[] = {ONE_WIRE_BUS_PIN_LINE_A,
                          ONE_WIRE_BUS_PIN_LINE_B};
const int oneWireCount = sizeof(oneWireLines) / sizeof(OneWire);
DallasTemperature sensor[oneWireCount];

// ===============================================================
// ====================> ETHERNET SETTINGS  <=====================
// ===============================================================
byte mac[] = {0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED};
IPAddress ipArduino(192, 168, 100, 216);  // Local Address
IPAddress gateway(192, 168, 100, 1);      // Local Gateway
IPAddress subnet(255, 255, 255, 0);       // Local Subnet
IPAddress ipPLC(192, 168, 100, 210);      // Remote PLC Address

//IPAddress ipArduino(192,168,51,200);                  // Local Address
//IPAddress ipPLC(192,168,51,199);                      // PLC Address
//IPAddress gateway(192, 168, 51, 100);
//IPAddress subnet(255, 255, 255, 0);

/*-----( Declare Variables )-----*/
int DBNum = 251; // This DB must be present in your PLC
byte Buffer[1024];
unsigned long Elapsed; // To calc the execution time

// S7Client will create an EthernetClient as TCP Client
S7Client Client;

void setup() /****** SETUP: RUNS ONCE ******/
{

  // start serial port to show results
  Serial.begin(9600);
  Serial.println("-------- INITIALIZATION STARTED --------");

  // Reset TWI interface sensors
  pinMode(4, OUTPUT);
  digitalWrite(4, HIGH);
  Serial.println("NIC Reset");
  digitalWrite(8, LOW);
  delay(1000);
  digitalWrite(8, HIGH);
  delay(2000);

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
    getDeviceAddress(i);
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
  
  delay(1000);
  Serial.println(" ");
  Serial.println(" ");
  Serial.println("=========================================================");
  Serial.println("================= START READING DATA ====================");

} //--(end setup )---

void Dump(void *Buffer, int Length);
void CheckError(int ErrNo);

int Watchdog;
const long interval_serial = 10000, interval_s7 = 2000;
unsigned long previousMillis_serial = 0, previousMillis_s7 = 0;

void loop() /****** LOOP: RUNS CONSTANTLY ******/
{
  int Size, Result;
  void *Target;
  unsigned long currentMillis = millis();
  
  if (currentMillis - previousMillis_serial >= interval_serial) {
    previousMillis_serial = currentMillis;
    Size = 64;
    Target = &Buffer; // Uses a larger buffer
  
    Serial.println();
  
    if (sensor[0].getDeviceCount())
    {
      sensor[0].requestTemperatures();
    }
    if (sensor[1].getDeviceCount())
    {
      sensor[1].requestTemperatures();
    }
    delay(1000);
  
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
 
    Serial.println(" ");
    Serial.println(" ");
    Serial.println("================= END READING DATA ====================");
    Serial.println("=========================================================");
  }
  
   if (currentMillis - previousMillis_s7 >= interval_s7) {
    previousMillis_s7 = currentMillis;
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
  
      Watchdog += 1;
      if (Watchdog > 100) {
        Watchdog = 0;
        }
      S7.SetIntAt(Target, 0, Watchdog);
  
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
  
      
      // Write PLC DBxxx
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
      }
      else
        CheckError(Result);
  
      //delay(500);
    }
    else //((!Client.Connected) or (!Connect()))
    {
      delay(500);
      Serial.println("PLC not connected");
    }
   
  Serial.println("<<<<<<<<<<<<<<<<<<<< END MAIN LOOP >>>>>>>>>>>>>>>>>>");
  }

} //--(end main loop )---

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
