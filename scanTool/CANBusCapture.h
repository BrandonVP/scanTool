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
extern void drawCapture();
extern void drawCaptureSelected();
extern void drawCaptureSource();
extern void drawCaptureOutput();
extern void CaptureButtons();
extern void drawReadInCANLCD();
extern void readInCANMsg(uint8_t);
extern void drawCANSerial();
extern void drawSendFrame(uint8_t);
extern void sendFrameButtons(uint8_t);
extern void setData(uint8_t);
extern void sendCANFrame(uint8_t);
extern void baudButtons();
extern void drawBaudScroll();
extern void drawCurrentBaud();
extern void drawBaud();
extern bool drawFilterMask();
extern void filterMaskButtons();
extern uint8_t setFilterMask(uint32_t&);
extern void filterMask();
extern void drawCANLog();
extern void drawCANLogScroll();
extern void CANLogButtons();
extern void findBaud();
#endif

