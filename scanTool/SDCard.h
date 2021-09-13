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

typedef char MyArray[10][13];

class SDCard
{
 protected:
	 struct files
	 {
		 char name[20];
	 }list[2];

 public:
	bool startSD();
	void createDRIVE(char*);
	void readFile(char* filename, uint8_t* arrayIn);
	void writeFile(char*, int, int);
	void writeFile(char*, String);
	void writeFileln(char*);
	void deleteFile(char*);
	void readLogFile(char*);
	uint8_t printDirectory(File dir, MyArray&);
};
#endif

