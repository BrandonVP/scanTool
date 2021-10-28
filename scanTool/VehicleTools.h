// VehicleTools.h

#ifndef _VEHICLETOOLS_h
#define _VEHICLETOOLS_h

#if defined(ARDUINO) && ARDUINO >= 100
	#include "arduino.h"
#else
	#include "WProgram.h"
#endif
extern bool drawVehicleTools();
extern void VehicleToolButtons();
extern void drawPIDSCAN();
extern void startPIDSCAN();
extern void drawPIDStreamScroll();
extern void drawPIDStream();
extern void PIDStreamButtons();
extern void PIDGauges();
extern void drawVIN();
extern void clearDTC();
extern void drawClearDTC();
extern uint8_t DTCButtons();
#endif

