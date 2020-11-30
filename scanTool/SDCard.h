// SDCard.h
#include <SD.h>

#ifndef _SDCard_h
#define _SDCard_h

#if defined(ARDUINO) && ARDUINO >= 100
	#include "arduino.h"
#else
	#include "WProgram.h"
#endif
class CANBus;

class SDCard
{
 protected:


 public:
	bool startSD();
	void writeFile(String, uint8_t, int);
	void writeFile(String, String);
	void writeFile(String, int);
	void writeFileln(String);
	void deleteFile(String);
	size_t readField(File*, char*, size_t, const char*);
	void readFile(String);
};

#endif

