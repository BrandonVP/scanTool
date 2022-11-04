/*
 ===========================================================================
 Name        : PolarisTools.h
 Author      : Brandon Van Pelt
 Created	 : 
 Description : UTV CAN Bus tools
 ===========================================================================
 */

#include "common.h"

#ifndef _POLARISTOOLS_H
#define _POLARISTOOLS_H

#if defined(ARDUINO) && ARDUINO >= 100
	#include "arduino.h"
#else
	#include "WProgram.h"
#endif

#ifdef _POLARISTOOLS_C

bool drawRZRTOOL();
void RZRToolButtons();

#else

extern bool drawRZRTOOL();
extern void RZRToolButtons();

#endif
#endif

