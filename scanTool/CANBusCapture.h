// CANBusCapture.h

#ifndef _CANBUSCAPTURE_h
#define _CANBUSCAPTURE_h

#if defined(ARDUINO) && ARDUINO >= 100
	#include "arduino.h"
#else
	#include "WProgram.h"
#endif
extern bool drawCANBus();
extern void CANBusButtons();
extern bool drawCapture();
extern void drawCaptureSelected();
extern bool drawCaptureSource();
extern bool drawCaptureOutput();
extern void CaptureButtons();
extern void drawReadInCANLCD();
extern void readInCANMsg(uint8_t);
extern bool drawSendFrame(uint8_t);
extern void sendFrameButtons(uint8_t);
extern void setData(uint8_t);
extern void sendCANFrame(uint8_t);
extern void baudButtons();
extern void drawBaudScroll();
extern void drawCurrentBaud();
extern bool drawBaud();
extern bool drawFilterMask();
extern void filterMaskButtons();
extern uint8_t setFilterMask(uint32_t&);
extern void filterMask();
extern bool drawCANLog();
extern void drawCANLogScroll();
extern void CANLogButtons();
extern void findBaud();
extern void openAllTraffic();
extern bool drawTimedTX();
#endif

