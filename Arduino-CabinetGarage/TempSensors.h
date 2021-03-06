//01: 0x28, 0x03, 0x3F, 0x79, 0xA2, 0x15, 0x03, 0xDF
//02: 0x28, 0xCE, 0xBB, 0x79, 0xA2, 0x16, 0x03, 0x8B
//03: 0x28, 0xE8, 0x5A, 0x79, 0xA2, 0x16, 0x03, 0x98
//04: 0x28, 0xCD, 0x8C, 0x79, 0xA2, 0x16, 0x03, 0x07
//05: 0x28, 0x85, 0x5A, 0x79, 0xA2, 0x15, 0x03, 0x44
//06: 0x28, 0x73, 0x5F, 0x79, 0xA2, 0x16, 0x03, 0x03
//07: 0x28, 0xEE, 0x1B, 0x79, 0xA2, 0x16, 0x03, 0x0E
//08: 0x28, 0x95, 0xD4, 0x79, 0xA2, 0x16, 0x03, 0x23
//09: 0x28, 0x0C, 0xD4, 0xDC, 0x0A, 0x00, 0x00, 0x5B
//10: 0x28, 0xFF, 0x2F, 0xBE, 0x6D, 0x18, 0x01, 0x2A
//11: 0x28, 0xFF, 0x0A, 0xAA, 0x6D, 0x18, 0x01, 0x36
//12: 0x28, 0xFF, 0x60, 0x95, 0x6D, 0x18, 0x01, 0x48

#define NUM_SENSORS_LINE_A 9 // the number of DS18B20 sensors on the 1-Wire bus A.
#define NUM_SENSORS_LINE_B 1 // the number of DS18B20 sensors on the 1-Wire bus B.
#define NUM_SENSORS_LINE_C 1 // the number of DS18B20 sensors on the 1-Wire bus C.
#define NUM_SENSORS_LINE_D 1 // the number of DS18B20 sensors on the 1-Wire bus D.

/*-----( Declare Constants )-----*/
// Pass our oneWire reference to Dallas Temperature.
const DeviceAddress Sensor_Address_Line_A[NUM_SENSORS_LINE_A] =
    {
        {0x28, 0x0C, 0xBE, 0xE8, 0x00, 0x00, 0x00, 0xDB},
        {0x28, 0x1E, 0x78, 0xFE, 0x08, 0x00, 0x00, 0x44},
        {0x28, 0xFF, 0xCF, 0x7D, 0x86, 0x16, 0x04, 0xA4},
        {0x28, 0xFF, 0x09, 0x5F, 0x87, 0x16, 0x05, 0x98},
        {0x28, 0xFF, 0x3A, 0x58, 0x86, 0x16, 0x04, 0x21},
        {0x28, 0xFF, 0x7F, 0x45, 0x87, 0x16, 0x05, 0x4A},
        {0x28, 0x0C, 0xD4, 0xDC, 0x0A, 0x00, 0x00, 0x5B}, //no.07
        {0x28, 0x77, 0x0B, 0x79, 0xA2, 0x15, 0x03, 0x00}, //parter maly pokoj
                                                          //  { 0x28, 0xDE, 0x13, 0x79, 0xA2, 0x15, 0x03, 0x88 },     //parter lazienka
                                                          //  { 0x28, 0xFF, 0xFB, 0xF9, 0x6D, 0x18, 0x01, 0x0D },     //parter lazienka
};
const DeviceAddress Sensor_Address_Line_B[NUM_SENSORS_LINE_B] =
    {
        {0x28, 0xDE, 0x13, 0x79, 0xA2, 0x15, 0x03, 0x88}, //parter lazienka
                                                          //  { 0x28, 0xFF, 0xFB, 0xF9, 0x6D, 0x18, 0x01, 0x0D },     //parter lazienka
};
const DeviceAddress Sensor_Address_Line_C[NUM_SENSORS_LINE_C] =
    {
        {0x28, 0xDE, 0x13, 0x79, 0xA2, 0x15, 0x03, 0x88}, //parter lazienka
                                                          //  { 0x28, 0xFF, 0xFB, 0xF9, 0x6D, 0x18, 0x01, 0x0D },     //parter lazienka
};
const DeviceAddress Sensor_Address_Line_D[NUM_SENSORS_LINE_D] =
    {
        {0x28, 0xDE, 0x13, 0x79, 0xA2, 0x15, 0x03, 0x88}, //parter lazienka
                                                          //  { 0x28, 0xFF, 0xFB, 0xF9, 0x6D, 0x18, 0x01, 0x0D },     //parter lazienka
};
