/*
 ===========================================================================
 Name        : ExtraFunctions.h
 Author      : Brandon Van Pelt
 Created	 : 11/15/2020 8:27:18 AM
 Description : Custom user functions
 ===========================================================================
 */

#include "common.h"

#ifndef _EXTRAFUNCTIONS_H
#define _EXTRAFUNCTIONS_H

#if defined(ARDUINO) && ARDUINO >= 100
	#include "arduino.h"
#else
	#include "WProgram.h"
#endif

#ifdef _EXTRAFUNCTIONS_C

bool drawExtraFN();
void extraFNButtons();
void drawOBDSimulator();
void drawMSGSpam();
void drawMSGSpamMin();
void drawMSGSpamMax();
void drawMSGInterval();
void incID();
void sendMSGButtons();
void sendMSG();
void MSGSpam();
bool drawDongleSim();
void dongleSimButtons();
bool drawDongleSimFord();
void dongleSimButtonsFord();
void resetDongle();

#else

extern bool drawExtraFN();
extern void extraFNButtons();
extern bool drawDongleSim();
extern void dongleSimButtons();
extern bool drawDongleSimFord();
extern void dongleSimButtonsFord();
extern void drawOBDSimulator();
extern void drawMSGSpam();
extern void sendMSGButtons();
extern void MSGSpam();
extern void drawMSGSpamMin();
extern void drawMSGSpamMax();
extern void drawMSGInterval();

#endif // _EXTRAFUNCTIONS_C
#endif // _EXTRAFUNCTIONS_H

