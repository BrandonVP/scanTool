#pragma once
#ifndef COMMON_H
#define COMMON_H

#include <UTFT.h>
#include <DS3231.h>
#include <RTCDue.h>
#include <UTouch.h>
#include "CANBus.h"
#include "SDCard.h"

// Harware Objects
extern CANBus can1;
extern SDCard sdCard;
extern DS3231 rtc;

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

extern uint8_t controlPage;
extern uint8_t page;
extern bool hasDrawn;

extern int x, y;


extern const uint32_t hexTable[8];
extern const uint32_t baudRates[6];

extern uint8_t selectedChannelOut;
extern uint8_t graphicLoaderState;


extern void waitForIt(int x1, int y1, int x2, int y2);
extern void drawRoundBtn(int x_start, int y_start, int x_stop, int y_stop, String button, int backgroundColor, int btnBorderColor, int btnTxtColor, int align);
extern void drawSquareBtn(int x_start, int y_start, int x_stop, int y_stop, String button, int backgroundColor, int btnBorderColor, int btnTxtColor, int align);
extern int keypadButtons();
extern void drawKeypad();
#endif // EXTERN_H
