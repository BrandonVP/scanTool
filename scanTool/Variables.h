/*
 ===========================================================================
 Name        : Variables.h
 Author      : Brandon Van Pelt
 Created	 :
 Description : Memory saver
 ===========================================================================
 */

#include "common.h"

#ifndef _VARIABLES_H
#define _VARIABLES_H

#if defined(ARDUINO) && ARDUINO >= 100
	#include "arduino.h"
#else
	#include "WProgram.h"
#endif

#ifdef _VARIABLES_C

uint8_t g_var8[8] = { 0,0,0,0,0,0,0,0 };
uint8_t g_var8Lock = 0;
uint16_t g_var16[8] = { 0,0,0,0,0,0,0,0 };
uint8_t g_var16Lock = 0;
uint32_t g_var32[8] = { 0,0,0,0,0,0,0,0 };
uint8_t g_var32Lock = 0;

#else

extern uint8_t g_var8[8];
extern uint8_t g_var8Lock;
extern uint16_t g_var16[8];
extern uint8_t g_var16Lock;
extern uint32_t g_var32[8];
extern uint8_t g_var32Lock;

extern bool isVar8Unlocked(uint8_t);
extern bool isVar16Unlocked(uint8_t);
extern bool isVar32Unlocked(uint8_t);
extern bool lockVar8(uint8_t);
extern bool lockVar16(uint8_t);
extern bool lockVar32(uint8_t);
extern bool unlockVar8(uint8_t);
extern bool unlockVar16(uint8_t);
extern bool unlockVar32(uint8_t);

#endif // _VARIABLES_C
#endif // _VARIABLES_H

