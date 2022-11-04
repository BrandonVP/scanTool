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

 // Big Endian
#define LOCK0 0x80
#define LOCK1 0x40
#define LOCK2 0x20
#define LOCK3 0x10
#define LOCK4 0x08
#define LOCK5 0x04
#define LOCK6 0x02
#define LOCK7 0x01

#define POS0 7
#define POS1 6
#define POS2 5
#define POS3 4
#define POS4 3
#define POS5 2
#define POS6 1
#define POS7 0

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

