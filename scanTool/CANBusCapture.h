/*
 ===========================================================================
 Name        : CANBusCapture.h
 Author      : Brandon Van Pelt
 Created	 : 11/15/2020 8:54:18 AM
 Description : CAN Bus capture and monitor tools
 ===========================================================================
 */

#ifndef _CANBUSCAPTURE_H
#define _CANBUSCAPTURE_H

#if defined(ARDUINO) && ARDUINO >= 100
	#include "arduino.h"
#else
	#include "WProgram.h"
#endif

#define TIMED_TX_MAX_SIZE 20

struct MessageRX
{
	char name[9];
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
	MessageRX node[20];
};

#ifdef _CANBUSCAPTURE_C

const size_t N = 2;

SchedulerRX RXtimedMSG;
uint8_t displayedNodePosition[5];

bool drawCANBus();
void CANBusButtons();
bool drawCapture();
bool drawCaptureSelected();
bool drawCaptureOutput();
bool drawCaptureSource();
void CaptureButtons();
void drawReadInCANLCD();
void hex2String(uint8_t value, char(&result)[N]);
void readInCANMsg(uint8_t channel);
void drawSendChannel(uint8_t channel);
void loadRXMsg();
void saveRXMsg();
bool drawTimedTX();
bool drawTXNode(uint8_t index);
bool drawEditTXNode(uint8_t node);
void deleteNode(uint8_t node);
void timedTXButtons();
void timedTX();
void editTXNodeButtons();
uint8_t findFreeTXNode();
void timedTXSend();
void TXSend(uint8_t msgNum);
bool drawBaud();
void drawCurrentBaud();
void drawBaudScroll();
void baudButtons();
void findBaud();
bool drawFilterMask();
void filterMaskButtons();
void openAllTraffic();
void filterMask();
bool drawCANLog();
void drawCANLogScroll();
void logDeleteConfirmation();
void CANLogButtons();
void playback();

#else

extern bool drawCANBus();
extern void CANBusButtons();
extern bool drawCapture();
extern bool drawCaptureSelected();
extern bool drawCaptureSource();
extern bool drawCaptureOutput();
extern void CaptureButtons();
extern void drawReadInCANLCD();
extern void readInCANMsg(uint8_t);
extern void baudButtons();
extern void drawBaudScroll();
extern void drawCurrentBaud();
extern bool drawBaud();
extern bool drawFilterMask();
extern void filterMask();
extern bool drawCANLog();
extern void drawCANLogScroll();
extern bool drawTimedTX();
extern bool drawTXNode(uint8_t);
extern void timedTX();
extern void timedTXSend();
extern void loadRXMsg();
extern void playback();

#endif // _CANBUSCAPTURE_C
#endif // _CANBUSCAPTURE_H

