// Settings.h

#ifndef _SETTINGS_h
#define _SETTINGS_h

#if defined(ARDUINO) && ARDUINO >= 100
	#include "arduino.h"
#else
	#include "WProgram.h"
#endif
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

extern bool drawSettings();
extern bool drawMACAddress();
extern bool drawAbout();
extern void settingsButtons();
extern void memoryUse();
extern void connectDongle();
extern void loadMACs();
#endif

