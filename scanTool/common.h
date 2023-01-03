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
#include "definitions.h"
#include "KeyInput.h"
#include "DueOverclock.h"

#define ECU_RX 0x7E8
#define PID_LIST_TX 0x7DF

// This defines the char length of a formatted message
// Inpacts reading and writing from SD card class and message streaming from the canbus class
#define MSG_STRING_LENGTH       (67)
#define SD_CAPTURE_NUM_MSG      (10)
#define SD_CAPTURE_BLOCK_SIZE   (SD_CAPTURE_NUM_MSG * MSG_STRING_LENGTH) // SD_CAPTURE_NUM_MSG * MSG_STRING_LENGTH


//#define DEBUG(x)
//#define DEBUG(x)  Serial.println(x);
#define DEBUG_HEX(x, z)  SerialUSB.print(x, z);
#define DEBUG(x)  SerialUSB.print(x);

//#define DEBUG_ERROR(x)
//#define DEBUG_ERROR(x)  Serial.println(x);
#define DEBUG_ERROR(x)  SerialUSB.println(x);

// CAN Bus message configuration
#define CAN0			1 // Messages on CAN0
#define CAN1			2 // Messages on CAN1
#define BOTH			3 // Messages on CAN0 and CAN1
#define BRIDGE_CAN1_RX  4 // Bridge CAN0 and CAN1 but only show CAN1 RX messages (One way capture)
#define BRIDGE_BOTH		5 // Bridge CAN0 and CAN1 and show both messages
#define WIFI			6 // Messages over WIFI

typedef enum
{
    SWIPE_DOWN = 0x01,
    SWIPE_UP = 0x02,
    SWIPE_RIGHT = 0x03,
    SWIPE_LEFT = 0x04
}   SWIPE;

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
extern uint32_t CANWiFiFilter;
extern uint32_t CANWiFiMask;

extern const uint32_t hexTable[8];
extern const uint32_t baudRates[6];

extern uint8_t selectedChannelOut;
extern uint8_t selectedSourceOut;
extern uint8_t graphicLoaderState;

extern char fileList[20][13];
extern uint8_t arrayIn[80];
extern uint8_t scroll;
extern bool hasPID;
extern uint32_t timer2;
extern bool isSerialOut;
extern bool isSDOut;
extern bool isMSGSpam;

extern void menuButtons();
extern void waitForIt(int x1, int y1, int x2, int y2);
extern void drawRoundBtn(int x_start, int y_start, int x_stop, int y_stop, String button, int backgroundColor, int btnBorderColor, int btnTxtColor, int align);
extern void drawSquareBtn(int x_start, int y_start, int x_stop, int y_stop, String button, int backgroundColor, int btnBorderColor, int btnTxtColor, int align);
extern void waitForItRect(int x1, int y1, int x2, int y2);
extern int keypadButtons();
extern void drawKeypad();
extern bool loadBar(int);
extern uint8_t errorMSGButton(uint8_t, uint8_t, uint8_t);
extern void drawErrorMSG2(String, String, String);
extern void drawErrorMSG(String, String, String);
extern void saveRamStates(uint32_t MaxUsedHeapRAM, uint32_t MaxUsedStackRAM, uint32_t MaxUsedStaticRAM, uint32_t MinfreeRAM);
extern bool Touch_getXY();
extern uint8_t keypadController(uint8_t&, uint16_t&);
extern uint8_t keypadControllerDec(uint8_t&, uint16_t&);
extern void backgroundProcess();
extern void drawKeypadDec();
extern void resetKeypad();
extern uint8_t keyboardController(uint8_t&);
extern void drawkeyboard();
extern char keyboardInput[9];
extern uint8_t swipe(uint32_t& lastTouch, uint8_t& initiated, uint16_t& initial_x, uint16_t& initial_y, uint16_t& last_x, uint16_t& last_y);

#endif // EXTERN_H
