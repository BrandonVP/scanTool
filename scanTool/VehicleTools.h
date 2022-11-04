/*
 ===========================================================================
 Name        : VehicleTools.h
 Author      : Brandon Van Pelt
 Created	 :
 Description : CAN Bus related vehicle tools
 ===========================================================================
 */

#include "common.h"

#ifndef _VEHICLETOOLS_H
#define _VEHICLETOOLS_H

#if defined(ARDUINO) && ARDUINO >= 100
	#include "arduino.h"
#else
	#include "WProgram.h"
#endif

#ifdef _VEHICLETOOLS_C

bool drawVehicleTools();
void VehicleToolButtons();
bool drawPIDSCAN();
void startPIDSCAN();
void drawPIDStreamScroll();
void drawPIDStream();
void PIDStreamButtons();
void streamPIDS();
bool drawPIDGauges();
void PIDGauges();
void drawVIN();
void clearDTC();
void drawClearDTC();
uint8_t DTCButtons();

#else

extern bool drawVehicleTools();
extern void VehicleToolButtons();
extern bool drawPIDSCAN();
extern void startPIDSCAN();
extern void drawPIDStream();
extern void PIDGauges();
extern void drawVIN();
extern void clearDTC();
extern void drawClearDTC();
extern bool drawPIDGauges();
extern uint8_t DTCButtons();

#endif // _VEHICLETOOLS_C
#endif // _VEHICLETOOLS_H

