// Settings.h

#ifndef _SETTINGS_H
#define _SETTINGS_H

#if defined(ARDUINO) && ARDUINO >= 100
#include "arduino.h"
#else
#include "WProgram.h"
#endif

#define CLOCK_72MHZ 72
#define CLOCK_78MHZ 78
#define CLOCK_84MHZ 84
#define CLOCK_90MHZ 90
#define CLOCK_96MHZ 96
#define CLOCK_102MHZ 102
#define CLOCK_108MHZ 108
#define CLOCK_114MHZ 114


struct dongleSettings
{
	char name[9];
	uint8_t data[6] = { 0, 0, 0, 0, 0, 0 };
	bool isOn = false;
	bool isDel = true;
};

struct savedMACs
{
	uint8_t nodeLength = 0;
	dongleSettings node[10];
};

#ifdef _SETTINGS_C

uint8_t MCUClockSpeed = CLOCK_84MHZ;

#else

extern uint8_t MCUClockSpeed;
extern bool drawSettings();
extern bool drawMACAddress();
extern bool drawAbout();
extern void settingsButtons();
extern void memoryUse();
extern void connectDongle();
extern void loadMACs();
extern void clockSpeedButtons();
extern bool drawClockSpeed();

#endif // _SETTINGS_C
#endif // _SETTINGS_H

