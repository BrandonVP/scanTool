/*
 ===========================================================================
 Name        : SDCard.h
 Author      : Brandon Van Pelt
 Created	 :
 Description : SDCard class manages the SD card reader hardware
 ===========================================================================
 */

#include <SD.h>
#include "CANBusCapture.h"
#include "Settings.h"

#ifndef _SDCard_h
#define _SDCard_h

#if defined(ARDUINO) && ARDUINO >= 100
	#include "arduino.h"
#else
	#include "WProgram.h"
#endif

#define SD_CARD_CS 8
#define ROW_DIM 100

class SDCard
{
 protected:
	 bool canDir = false;
	 typedef char MyArray[20][13];
 public:
	void setSDFilename(char* );
	void writeFileS(char*, uint8_t);
	bool startSD();
	void createDRIVE(char*);
	void readFile(char* filename, uint8_t* arrayIn);
	void writeFile(char*, int, int);
	void writeFile(char*, String);
	void writeFileln(char*);
	void writeSendMsg(SchedulerRX);
	void readSendMsg(SchedulerRX &);
	void deleteFile(char*);
	void readLogFile(char*);
	uint32_t fileLength(char*);
	void tempCopy(char* filename);
	void split(char*, uint32_t);
	uint32_t fileSize(char* );
	uint8_t printDirectory(File dir, MyArray&);
	void writeMACs(savedMACs);
	void readMAC(savedMACs& msgStruct);
	void readLogFileLCD(char* filename, uint32_t&, bool);
};
#endif

