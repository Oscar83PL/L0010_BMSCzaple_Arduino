
#define NUM_SENSORS_LINE_A 1 // the number of DS18B20 sensors on the 1-Wire bus A.
#define NUM_SENSORS_LINE_B 1 // the number of DS18B20 sensors on the 1-Wire bus B.

/*-----( Declare Constants )-----*/
// Pass our oneWire reference to Dallas Temperature.
const DeviceAddress Sensor_Address_Line_A[NUM_SENSORS_LINE_A] =
    {
        0x28, 0x03, 0x3F, 0x79, 0xA2, 0x15, 0x03, 0xDF,  //01: Garderoba
};
const DeviceAddress Sensor_Address_Line_B[NUM_SENSORS_LINE_B] =
    {
        {0x28, 0xDE, 0x13, 0x79, 0xA2, 0x15, 0x03, 0x88}, //parter lazienka

};
