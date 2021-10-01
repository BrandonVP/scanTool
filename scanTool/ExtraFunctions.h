// ExtraFunctions.h

#ifndef _EXTRAFUNCTIONS_h
#define _EXTRAFUNCTIONS_h

#if defined(ARDUINO) && ARDUINO >= 100
	#include "arduino.h"
#else
	#include "WProgram.h"
#endif
extern void drawExtraFN();
extern void extraFNButtons();
extern void drawDongleSim();
extern void dongleSimButtons();
extern void drawDongleSimFord();
extern void dongleSimButtonsFord();
extern void resetDongle();
#endif

