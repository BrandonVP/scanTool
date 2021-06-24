// SDCard.h
#include <SD.h>
#include "CANBus.h"

#ifndef _SDCard_h
#define _SDCard_h

#if defined(ARDUINO) && ARDUINO >= 100
	#include "arduino.h"
#else
	#include "WProgram.h"
#endif

#define SD_CARD_CS 8
#define ROW_DIM 100

class CANBus;

class SDCard
{
 protected:
	 

 public:
	bool startSD();
	void createDRIVE(char*);
	void readFile(char* filename, uint8_t* arrayIn);
	void writeFile(char*, int, int);
	void writeFile(char*, String);
	void writeFileln(char*);
	void deleteFile(char*);
	void readLogFile();
	void readLogFile2();
	//size_t readField(File*, char*, size_t, const char*);
};
#endif

