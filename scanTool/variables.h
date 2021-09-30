#pragma once




// Holds CAN Bus capture replay filenames
char fileList[10][13];

// Determines if a PID scan was performed before displaying pid list
bool hasPID = false;

// Holds PIDS for the pidscan function
uint8_t arrayIn[80];

// TODO: Replace scroll with var
uint8_t scroll = 0;



// Filter range / Filter Mask
uint32_t CAN0Filter = 0x000;
uint32_t CAN0Mask = 0xFFF;
uint32_t CAN1Filter = 0x000;
uint32_t CAN1Mask = 0xFFF;



// Use to load pages in pieces to prevent blocking while loading entire page
uint8_t graphicLoaderState = 0;

// TODO: This uses a lot of memory for a simple graphic function
uint32_t waitForItTimer = 0;
uint16_t x1_ = 0;
uint16_t y1_ = 0;
uint16_t x2_ = 0;
uint16_t y2_ = 0;
bool isWaitForIt = false;

