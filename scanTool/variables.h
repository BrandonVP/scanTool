#pragma once
// Used for page control
uint8_t controlPage = 0;
uint8_t page = 0;
bool hasDrawn = false;

// For touch controls
int x, y;

// Holds CAN Bus capture replay filenames
char fileList[10][13];

// Determines if a PID scan was performed before displaying pid list
bool hasPID = false;

// Holds PIDS for the pidscan function
uint8_t arrayIn[80];

// TODO: Replace scroll with var
uint8_t scroll = 0;

// *Used by background process*
uint8_t selectedChannelOut = 0;
uint32_t timer2 = 0;
bool isSerialOut = false;

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

const uint32_t baudRates[6] = { 1000000, 800000, 500000, 250000, 125000, 100000 };