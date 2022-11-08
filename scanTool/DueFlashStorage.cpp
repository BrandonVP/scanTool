#include "DueFlashStorage.h"

DueFlashStorage::DueFlashStorage() {
  uint32_t retCode;

  /* Initialize flash: 6 wait states for flash writing. */
  retCode = flash_init(FLASH_ACCESS_MODE_128, 6);
  if (retCode != FLASH_RC_OK) {
    _FLASH_DEBUG("Flash init failed\n");
  }
}

byte DueFlashStorage::read(uint32_t address, int block) {
  if (block == 0) return FLASH_START[address];
  else return FLASH_START[address + IFLASH0_SIZE];
}

byte* DueFlashStorage::readAddress(uint32_t address, int block) {
  if (block == 0) return FLASH_START+address;
  else return FLASH_START+address+IFLASH0_SIZE;
}

boolean DueFlashStorage::write(uint32_t address, byte value, int block) {
  uint32_t retCode;
  uint32_t byteLength = 1;  
  byte *data;

  if (block != 0) address += IFLASH0_SIZE;

  retCode = flash_unlock((uint32_t)FLASH_START+address, (uint32_t)FLASH_START+address + byteLength - 1, 0, 0);
  if (retCode != FLASH_RC_OK) {
    _FLASH_DEBUG("Failed to unlock flash for write\n");
    return false;
  }

  // write data
  retCode = flash_write((uint32_t)FLASH_START+address, &value, byteLength, 1);
  //retCode = flash_write((uint32_t)FLASH_START, data, byteLength, 1);

  if (retCode != FLASH_RC_OK) {
    _FLASH_DEBUG("Flash write failed\n");
    return false;
  }

  // Lock page
  retCode = flash_lock((uint32_t)FLASH_START+address, (uint32_t)FLASH_START+address + byteLength - 1, 0, 0);
  if (retCode != FLASH_RC_OK) {
    _FLASH_DEBUG("Failed to lock flash page\n");
    return false;
  }
  return true;
}

boolean DueFlashStorage::write(uint32_t address, byte *data, uint32_t dataLength, int block) {
  uint32_t retCode;

  if (block != 0) address += IFLASH0_SIZE;

  if ((uint32_t)FLASH_START+address >= (IFLASH1_ADDR + IFLASH1_SIZE)) {
    _FLASH_DEBUG("Flash write address too high\n");
    return false;
  }

  if (((uint32_t)FLASH_START+address & 3) != 0) {
    _FLASH_DEBUG("Flash start address must be on four byte boundary\n");
    return false;
  }

  SerialUSB.print("Write addr: ");
  SerialUSB.println((long)(FLASH_START + address), HEX);

  // Unlock page
  retCode = flash_unlock((uint32_t)FLASH_START+address, (uint32_t)FLASH_START+address + dataLength - 1, 0, 0);
  if (retCode != FLASH_RC_OK) {
    _FLASH_DEBUG("Failed to unlock flash for write\n");
    return false;
  }

  // write data
  retCode = flash_write((uint32_t)FLASH_START+address, data, dataLength, 1);

  if (retCode != FLASH_RC_OK) {
    _FLASH_DEBUG("Flash write failed\n");
    return false;
  }
  else
  {
	  _FLASH_DEBUG("Success in writing a page\n");
  }

  // Lock page
    retCode = flash_lock((uint32_t)FLASH_START+address, (uint32_t)FLASH_START+address + dataLength - 1, 0, 0);
  if (retCode != FLASH_RC_OK) {
    _FLASH_DEBUG("Failed to lock flash page\n");
    return false;
  }
  return true;
}

//False = Currently set to boot from FLASH0, True = Set to boot from FLASH1 instead
bool DueFlashStorage::getGPNVMBootMode()
{
	if (flash_is_gpnvm_set(2) == 0) return false;
	return true;
}

void DueFlashStorage::setGPNVMBootMode(bool mode)
{
	//SerialUSB.println("About to set how to boot");
	__disable_irq();

	//flash_clear_gpnvm(1); //boot from ROM for testing
	flash_set_gpnvm(1); //boot from Flash for production


	//SerialUSB.print("Set GPNVM Mode to ");
	//SerialUSB.println(mode);
	if (mode) flash_set_gpnvm(2);
	else flash_clear_gpnvm(2);
	//I think that this becomes active IMMEDIATELY so the sketch will pretty much be guaranteed to crash after this is run.

	__enable_irq();
}
