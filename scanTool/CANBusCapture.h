// CANBusCapture.h

#ifndef _CANBUSCAPTURE_h
#define _CANBUSCAPTURE_h

#if defined(ARDUINO) && ARDUINO >= 100
	#include "arduino.h"
#else
	#include "WProgram.h"
#endif

extern void drawCANBus();
extern void CANBusButtons();
extern void drawReadInCANLCD();
extern void readInCANMsg(uint8_t);
extern void drawCANSerial();
extern void drawSendFrame(uint8_t);
extern void sendFrameButtons(uint8_t);
extern void setData(uint8_t);
extern void sendFrame(uint8_t);
#endif

