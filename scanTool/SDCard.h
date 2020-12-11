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
	void createDRIVE(char*);
	uint8_t* readFile(char* filename, uint8_t* arrayIn);
	void writeFile(char*, int, int);
	void writeFile(char*, String);
	void writeFileln(char*);
	void deleteFile(char*);
	size_t readField(File*, char*, size_t, const char*);
};
#endif

