// ExtraFunctions.h

#ifndef _EXTRAFUNCTIONS_h
#define _EXTRAFUNCTIONS_h

#if defined(ARDUINO) && ARDUINO >= 100
	#include "arduino.h"
#else
	#include "WProgram.h"
#endif
extern bool drawExtraFN();
extern void extraFNButtons();
extern bool drawDongleSim();
extern void dongleSimButtons();
extern bool drawDongleSimFord();
extern void dongleSimButtonsFord();
extern void resetDongle();
extern void drawOBDSimulator();
extern void drawMSGSpam();
extern void sendMSGButtons();
extern void MSGSpam();
extern void sendMSG();
extern void drawMSGSpamMin();
extern void drawMSGSpamMax();
extern void drawMSGInterval();
#endif

