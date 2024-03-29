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

#define CLEAR_DTC_RESET 12000 // MS counter to reset DTC page back to default

 // Determines if a PID scan was performed before displaying pid list
bool hasPID = false;

// Holds PIDS for the pidscan function
uint8_t arrayIn[80];

bool drawVehicleTools();
void VehicleToolButtons();
bool drawPIDSCAN();
void startPIDSCAN();
void drawPIDStreamScroll();
bool drawPIDStream();
void PIDStreamButtons();
void streamPIDS();
bool drawPIDGauges();
void PIDGauges();
void drawVIN();
void clearDTC();
bool drawDTC();
void DTC();
uint8_t DTCButtons();

#else

// Determines if a PID scan was performed before displaying pid list
extern bool hasPID;

// Holds PIDS for the pidscan function
extern uint8_t arrayIn[80];

extern bool drawVehicleTools();
extern void VehicleToolButtons();
extern bool drawPIDSCAN();
extern void startPIDSCAN();
extern bool drawPIDStream();
extern void PIDGauges();
extern void drawVIN();
extern void clearDTC();
extern bool drawDTC();
extern bool drawPIDGauges();
extern uint8_t DTCButtons();
extern void DTC();
extern void drawPIDStreamScroll();
extern void streamPIDS();
extern void PIDStreamButtons();

#endif // _VEHICLETOOLS_C
#endif // _VEHICLETOOLS_H

