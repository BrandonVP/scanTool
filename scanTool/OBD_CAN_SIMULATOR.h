#pragma once
#include "common.h"
#include "CANBus.h"
#include <due_can.h>

#ifndef _OBD_CAN_SIMULATOR_h
#define _OBD_CAN_SIMULATOR_h

#if defined(ARDUINO) && ARDUINO >= 100
#include "arduino.h"
#else
#include "WProgram.h"
#endif
extern void entry();
extern void obdReq(byte* data);
extern void unsupported(byte mode, byte pid);
extern void negAck(byte mode, byte reason);
extern void unsupportedPrint(byte mode, byte pid);
extern void iso_tp(byte mode, byte pid, int len, byte* data);
#endif
