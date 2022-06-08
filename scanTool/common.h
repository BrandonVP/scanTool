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
#include "Variables.h"
#include "OBD_CAN_SIMULATOR.h"

// Big Endian
#define LOCK0 0x80
#define LOCK1 0x40
#define LOCK2 0x20
#define LOCK3 0x10
#define LOCK4 0x08
#define LOCK5 0x04
#define LOCK6 0x02
#define LOCK7 0x01

#define POS0 7
#define POS1 6
#define POS2 5
#define POS3 4
#define POS4 3
#define POS5 2
#define POS6 1
#define POS7 0

#define ECU_RX 0x7E8
#define PID_LIST_TX 0x7DF

//#define DEBUG(x)
//#define DEBUG(x)  Serial.println(x);
#define DEBUG_HEX(x, z)  SerialUSB.print(x, z);
#define DEBUG(x)  SerialUSB.print(x);

//#define DEBUG_ERROR(x)
//#define DEBUG_ERROR(x)  Serial.println(x);
#define DEBUG_ERROR(x)  SerialUSB.println(x);

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
extern bool isFinished;
extern uint8_t state;

extern uint8_t controlPage;
extern uint8_t page;
extern uint8_t nextPage;
extern bool hasDrawn;

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
extern bool isSDOut;
extern bool isMSGSpam;

extern void waitForIt(int x1, int y1, int x2, int y2);
extern void drawRoundBtn(int x_start, int y_start, int x_stop, int y_stop, String button, int backgroundColor, int btnBorderColor, int btnTxtColor, int align);
extern void drawSquareBtn(int x_start, int y_start, int x_stop, int y_stop, String button, int backgroundColor, int btnBorderColor, int btnTxtColor, int align);
extern void waitForItRect(int x1, int y1, int x2, int y2);
extern int keypadButtons();
extern void drawKeypad();
extern bool loadBar(int);
extern void drawErrorMSG(String title, String eMessage1, String eMessage2);
extern void saveRamStates(uint32_t MaxUsedHeapRAM, uint32_t MaxUsedStackRAM, uint32_t MaxUsedStaticRAM, uint32_t MinfreeRAM);
extern bool Touch_getXY();
extern uint8_t keypadController(uint8_t&, uint16_t&);
extern uint8_t keypadControllerDec(uint8_t&, uint16_t&);
extern void backgroundProcess();
extern void drawKeypadDec();
extern void resetKeypad();
extern uint8_t keyboardController(uint8_t& index);
extern void drawkeyboard();
extern char keyboardInput[9];
#endif // EXTERN_H
