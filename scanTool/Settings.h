// Settings.h

#ifndef _SETTINGS_h
#define _SETTINGS_h

#if defined(ARDUINO) && ARDUINO >= 100
	#include "arduino.h"
#else
	#include "WProgram.h"
#endif
extern bool drawSettings();
extern bool drawMACAddress();
extern bool drawAbout();
extern void settingsButtons();
extern void memoryUse();
extern void connectDongle();
#endif

