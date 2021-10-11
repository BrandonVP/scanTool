// Variables.h

#ifndef _VARIABLES_h
#define _VARIABLES_h

#if defined(ARDUINO) && ARDUINO >= 100
	#include "arduino.h"
#else
	#include "WProgram.h"
#endif
//
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

#endif

