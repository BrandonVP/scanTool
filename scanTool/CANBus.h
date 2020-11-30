// CANBus.h
#include "SDCard.h"

#ifndef _CANBus_h
#define _CANBus_h

#if defined(ARDUINO) && ARDUINO >= 100
	#include "arduino.h"
#else
	#include "WProgram.h"
#endif
class SDCard;

class CANBus
{
 private:
	 SDCard SDPrint;
	 typedef byte test[8];
	 bool hasNextPID;
 public:

	CANBus();
	bool recordCAN(uint16_t);
	void setNextPID(bool);
	bool getNextPID();
	void getMessage(test&, int&);
	void sendData(uint16_t, byte*);
	void startCAN();
};
#endif

