// CANBusCapture.h

#ifndef _CANBUSCAPTURE_h
#define _CANBUSCAPTURE_h

#if defined(ARDUINO) && ARDUINO >= 100
	#include "arduino.h"
#else
	#include "WProgram.h"
#endif

#define TIMED_TX_MAX_SIZE 10

struct MessageRX
{
	uint8_t channel = 0;
	uint16_t interval = 0;
	uint32_t id = 0;
	uint32_t timer = 0;
	uint8_t data[8] = {0, 0, 0, 0, 0, 0, 0, 0};
	bool isOn = false;
	bool isDel = true;
};


struct SchedulerRX
{
	uint8_t nodeLength = 0;
	MessageRX node[10];
};

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
extern bool drawTXNode(uint8_t);
extern void timedTXButtons();
extern void timedTX();
extern uint8_t findFreeTXNode();
extern bool drawEditTXNode(uint8_t);
extern void editTXNodeButtons();
extern void timedTXSend();
#endif

