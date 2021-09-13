#pragma once
// LCD display
//(byte model, int RS, int WR, int CS, int RST, int SER)
UTFT myGLCD(ILI9488_16, 7, 38, 9, 10);
//RTP: byte tclk, byte tcs, byte din, byte dout, byte irq
UTouch  myTouch(2, 6, 3, 4, 5);

// For touch controls
int x, y;

// Used for page control
uint8_t controlPage = 0;
uint8_t page = 0;
bool hasDrawn = false;

// *Used by background process*
uint8_t selectedChannelOut = 0;
uint32_t timer2 = 0;

// External import for fonts
extern uint8_t SmallFont[];
extern uint8_t BigFont[];

// Harware Objects
CANBus can1;
SDCard sdCard;
DS3231 rtc(SDA, SCL);

// Used PID functions
uint8_t arrayIn[80];

// TODO: Replace scroll with var
uint8_t scroll = 0;

// Performing a sucessful PID scan will change this to true
bool hasPID = false;

// Filter range / Filter Mask
uint32_t CAN0Filter = 0x000;
uint32_t CAN0Mask = 0xFFF;
uint32_t CAN1Filter = 0x000;
uint32_t CAN1Mask = 0xFFF;

// General use variables
// Any non-background process function can use
// Initialize to 0 before use
bool nextState = false;
bool isFinished = false;
bool isSerialOut = false;
uint8_t state = 0;
int16_t counter1 = 0;
uint16_t var1 = 0;
uint8_t var2 = 0;
uint16_t var3 = 0;
uint32_t var4 = 0;
uint32_t var5 = 0;
uint32_t timer1 = 0;

// Use to load pages in pieces to prevent blocking while loading entire page
uint8_t graphicLoaderState = 0;

// TODO: This uses a lot of memory for a simple graphic function
uint32_t waitForItTimer = 0;
uint16_t x1_ = 0;
uint16_t y1_ = 0;
uint16_t x2_ = 0;
uint16_t y2_ = 0;
bool isWaitForIt = false;

// Used for converting keypad input to appropriate hex place
const uint32_t hexTable[8] = { 1, 16, 256, 4096, 65536, 1048576, 16777216, 268435456 };

//
char fileList[10][13];