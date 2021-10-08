#pragma once
#ifndef COMMON_H
#define COMMON_H

#include <UTFT.h>
#include <DS3231.h>
#include <RTCDue.h>
#include <UTouch.h>
#include "CANBus.h"
#include "SDCard.h"
#include <due_can.h>

// Harware Objects
extern CANBus can1;
extern SDCard sdCard;
extern DS3231 rtc;

extern CAN_FRAME incCAN0;
extern CAN_FRAME incCAN1;
extern CAN_FRAME CANOut;

// LCD fonts
extern uint8_t SmallFont[];
extern uint8_t BigFont[];

// LCD display
//(byte model, int RS, int WR, int CS, int RST, int SER)
extern UTFT myGLCD;
//RTP: byte tclk, byte tcs, byte din, byte dout, byte irq
extern UTouch  myTouch;


// General use variables
// Any non-background process function can use
// Initialize to 0 before use
extern bool nextState;
extern bool isFinished;
extern uint8_t state;
extern int16_t counter1;
extern uint16_t var1;
extern uint8_t var2;
extern uint16_t var3;
extern uint32_t var4;
extern uint32_t var5;
extern uint32_t timer1;
extern uint8_t var6;
extern uint8_t controlPage;
extern uint8_t page;
extern bool hasDrawn;

// TODO: This uses a lot of memory for a simple graphic function
extern uint32_t waitForItTimer;
extern uint16_t x1_;
extern uint16_t y1_;
extern uint16_t x2_;
extern uint16_t y2_;
extern bool isWaitForIt;

extern int x, y;

// Filter range / Filter Mask
extern uint32_t CAN0Filter;
extern uint32_t CAN0Mask;
extern uint32_t CAN1Filter;
extern uint32_t CAN1Mask;


extern const uint32_t hexTable[8];
extern const uint32_t baudRates[6];

extern uint8_t selectedChannelOut;
extern uint8_t selectedSourceOut;
extern uint8_t graphicLoaderState;

extern char fileList[10][13];
extern uint8_t arrayIn[80];
extern uint8_t scroll;
extern bool hasPID;
extern uint32_t timer2;
extern bool isSerialOut;

extern void waitForIt(int x1, int y1, int x2, int y2);
extern void drawRoundBtn(int x_start, int y_start, int x_stop, int y_stop, String button, int backgroundColor, int btnBorderColor, int btnTxtColor, int align);
extern void drawSquareBtn(int x_start, int y_start, int x_stop, int y_stop, String button, int backgroundColor, int btnBorderColor, int btnTxtColor, int align);
extern void waitForItRect(int x1, int y1, int x2, int y2);
extern int keypadButtons();
extern void drawKeypad();
extern void loadBar(int);
extern void drawErrorMSG(String title, String eMessage1, String eMessage2);
void saveRamStates(uint32_t MaxUsedHeapRAM, uint32_t MaxUsedStackRAM, uint32_t MaxUsedStaticRAM, uint32_t MinfreeRAM);
#endif // EXTERN_H
