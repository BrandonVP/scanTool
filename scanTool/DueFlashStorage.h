/* 
DueFlashStorage saves non-volatile data for Arduino Due.
But, it has been modified for use as a firmware writer

Note: uploading new software will erase all flash so data written to flash
using this library will not survive a new software upload. 

Inspiration from Pansenti at https://github.com/Pansenti/DueFlash
Rewritten and modified by Sebastian Nilsson

Further modified by Collin Kidder so that it can easily
write firmware to either flash block
*/


#ifndef DUEFLASHSTORAGE_H
#define DUEFLASHSTORAGE_H

#include <Arduino.h>
#include "flash_efc.h"
#include "efc.h"

// 1Kb of data
#define DATA_LENGTH   ((IFLASH1_PAGE_SIZE/sizeof(byte))*4)

// choose a start address that's offset to show that it doesn't have to be on a page boundary
#define  FLASH_START  ((byte *)IFLASH0_ADDR)

//  FLASH_DEBUG can be enabled to get debugging information displayed.
#define FLASH_DEBUG

#ifdef FLASH_DEBUG
#define _FLASH_DEBUG(x) SerialUSB.print(x);
#else
#define _FLASH_DEBUG(x)
#endif

//  DueFlash is the main class for flash functions
class DueFlashStorage {
public:
  DueFlashStorage();
  
  // write() writes the specified amount of data into flash.
  // flashStart is the address in memory where the write should start
  // data is a pointer to the data to be written
  // dataLength is length of data in bytes
  
	byte read(uint32_t address, int block);
	byte* readAddress(uint32_t address, int block);
	boolean write(uint32_t address, byte value, int block);
	boolean write(uint32_t address, byte *data, uint32_t dataLength, int block);
	bool getGPNVMBootMode(); //true = boot from FLASH1, false = boot from FLASH0
	void setGPNVMBootMode(bool mode);
};

#endif
