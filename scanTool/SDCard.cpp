/*
 ===========================================================================
 Name        : SDCard.cpp
 Author      : Brandon Van Pelt
 Created	 :
 Description : SDCard class manages the SD card reader hardware
 ===========================================================================
 */

#include "SDCard.h"
#include "common.h"

// Called at setup to initialize the SD Card
bool SDCard::startSD()
{
	if (!SD.begin(SD_CARD_CS)) {
		return false;
	}
	return true;
}

/*=========================================================
	Write File Methods
===========================================================*/
// Write string to SD Card
void SDCard::writeFile(char* filename, String incoming)
{
	// File created and opened for writing
	File myFile = SD.open(filename, FILE_WRITE);

	// Check if file was sucsefully open
	if (myFile)
	{
		myFile.print(incoming);
		myFile.close();
	}
}

// 
void SDCard::setSDFilename(char* filename)
{
	for (uint8_t i = 0; i < 50; i++)
	{
		SDfilename[i] = '\0';
	}
	char* a1 = SDfilename;
	memcpy(a1, "canlog/", 7);
	strcat(SDfilename, filename);
	//SerialUSB.println(SDfilename);
}

// Write string to SD Card
void SDCard::writeFileS(char* incoming, uint8_t size)
{
	// File created and opened for writing
	File myFile = SD.open(SDfilename, FILE_WRITE);

	// Check if file was sucsefully open
	if (myFile)
	{
		myFile.write(incoming, (size * MSG_STRING_LENGTH));
		myFile.close();
	}
}

// Write integer and base to SD Card
void SDCard::writeFile(char* filename, int incoming, int base)
{
	// File created and opened for writing
	File myFile = SD.open(filename, FILE_WRITE);

	// Check if file was sucsefully open
	if (myFile)
	{
		myFile.print(incoming, base);
		myFile.close();
	}
}

// Write return to SD Card file
void SDCard::writeFileln(char* filename)
{
	// File created and opened for writing
	File myFile = SD.open(filename, FILE_WRITE);

	// Check if file was sucsefully open
	if (myFile)
	{
		myFile.println(" ");
		myFile.close();
	}
}

// Saves users CAN Bus RX messages
void SDCard::writeSendMsg(SchedulerRX msgStruct)
{
	char buffer[100];

	// Delete old before writing new file
	SD.remove("SYSTEM/CANMsg.txt");

	// File created and opened for writing
	File myFile = SD.open("SYSTEM/CANMsg.txt", FILE_WRITE);

	for (uint8_t i = 0; i < 20; i++)
	{
		if (strcmp(msgStruct.node[i].name, "\0"))
		{
			sprintf(buffer, "%s %x %x %x %x %x %x %x %x %x %x %x %x %x \n", msgStruct.node[i].name, msgStruct.node[i].channel, msgStruct.node[i].interval, msgStruct.node[i].id,
				msgStruct.node[i].data[0], msgStruct.node[i].data[1], msgStruct.node[i].data[2], msgStruct.node[i].data[3], msgStruct.node[i].data[4], msgStruct.node[i].data[5], msgStruct.node[i].data[6], msgStruct.node[i].data[7],
				msgStruct.node[i].isOn, msgStruct.node[i].isDel);
			//SerialUSB.println(msgStruct.node[i].name);
		}
		else
		{
			sprintf(buffer, "%s %x %x %x %x %x %x %x %x %x %x %x %x %x \n", "(null)", msgStruct.node[i].channel, msgStruct.node[i].interval, msgStruct.node[i].id,
				msgStruct.node[i].data[0], msgStruct.node[i].data[1], msgStruct.node[i].data[2], msgStruct.node[i].data[3], msgStruct.node[i].data[4], msgStruct.node[i].data[5], msgStruct.node[i].data[6], msgStruct.node[i].data[7],
				msgStruct.node[i].isOn, msgStruct.node[i].isDel);
			//SerialUSB.println("(null)");
		}
		
		// Copy buffer to file after confirming file was open
		if (myFile)
		{
			myFile.print(buffer);
		}
	}
	myFile.close();
}

// Saves users CAN Bus RX messages
void SDCard::writeMACs(savedMACs msgStruct)
{
	char buffer[100];

	// Delete old before writing new file
	SD.remove("SYSTEM/MAC.txt");

	// File created and opened for writing
	File myFile = SD.open("SYSTEM/MAC.txt", FILE_WRITE);

	for (uint8_t i = 0; i < 10; i++)
	{
		if (strcmp(msgStruct.node[i].name, "\0"))
		{
			sprintf(buffer, "%s %x %x %x %x %x %x %x %x \n", 
				msgStruct.node[i].name, msgStruct.node[i].data[0], msgStruct.node[i].data[1], msgStruct.node[i].data[2], msgStruct.node[i].data[3], msgStruct.node[i].data[4], msgStruct.node[i].data[5], msgStruct.node[i].isOn, msgStruct.node[i].isDel);
		}
		else
		{
			sprintf(buffer, "%s %x %x %x %x %x %x %x %x \n", 
				"(null)", msgStruct.node[i].data[0], msgStruct.node[i].data[1], msgStruct.node[i].data[2], msgStruct.node[i].data[3], msgStruct.node[i].data[4], msgStruct.node[i].data[5], msgStruct.node[i].isOn, msgStruct.node[i].isDel);
		}

		// Copy buffer to file after confirming file was open
		if (myFile)
		{
			myFile.print(buffer);
		}
	}
	myFile.close();
}


/*=========================================================
	Delete File Methods
===========================================================*/
// Delete SD Card file
void SDCard::deleteFile(char* filename)
{
	//SerialUSB.println(filename);
	//remove any existing file with this name
	SD.remove(filename);
}

/*=========================================================
	Read File Methods
===========================================================*/
// Reads in PID scan results file
void SDCard::readFile(char* filename, uint8_t* arrayIn)
{
	// File created and opened for writing
	File myFile = SD.open(filename, FILE_READ);
	uint32_t i = 0;
	String tempStr;
	char c[20];
	while (myFile.available())
	{
		tempStr = (myFile.readStringUntil('0x'));
		strcpy(c, tempStr.c_str());
		arrayIn[i] = strtol(c, NULL, 16);
		i++;
	}
	myFile.close();
}

// Reads users saved CAN Bus RX messages
void SDCard::readSendMsg(SchedulerRX& msgStruct)
{
	// File created and opened for writing
	File myFile = SD.open(F("SYSTEM/CANMsg.txt"), FILE_READ);

	if (myFile)
	{
		for (uint8_t i = 0; i < 20; i++)
		{
			char buffer[51];
			char s1[9];

			myFile.readBytesUntil('\n', buffer, 51);
			
			sscanf(buffer, "%s %x %x %x %x %x %x %x %x %x %x %x %x %x", s1, &msgStruct.node[i].channel, &msgStruct.node[i].interval, &msgStruct.node[i].id, 
				&msgStruct.node[i].data[0], &msgStruct.node[i].data[1], &msgStruct.node[i].data[2], &msgStruct.node[i].data[3], &msgStruct.node[i].data[4], &msgStruct.node[i].data[5], &msgStruct.node[i].data[6], &msgStruct.node[i].data[7],
				&msgStruct.node[i].isOn, &msgStruct.node[i].isDel);

			msgStruct.node[i].isOn = false;

			if (!strcmp(s1, "(null)"))
			{
				char *temp = '\0';
				strncpy(msgStruct.node[i].name, "\0", 9);
			}
			else if (s1 == NULL)
			{
				strncpy(msgStruct.node[i].name, "\0", 9);
			}
			else if (s1 == "\0")
			{
				strncpy(msgStruct.node[i].name, "\0", 9);
			}
			else
			{
				strncpy(msgStruct.node[i].name, s1, 9);
			}

			/*
			SerialUSB.print("Index: ");
			SerialUSB.println(i);
			SerialUSB.println(msgStruct.node[i].name);
			SerialUSB.println(msgStruct.node[i].channel);
			SerialUSB.println(msgStruct.node[i].interval);
			SerialUSB.println(msgStruct.node[i].id);
			SerialUSB.println(msgStruct.node[i].timer);
			SerialUSB.print("data: ");
			SerialUSB.print(msgStruct.node[i].data[0]);
			SerialUSB.print(" ");
			SerialUSB.print(msgStruct.node[i].data[1]);
			SerialUSB.print(" ");
			SerialUSB.print(msgStruct.node[i].data[2]);
			SerialUSB.print(" ");
			SerialUSB.print(msgStruct.node[i].data[3]);
			SerialUSB.print(" ");
			SerialUSB.print(msgStruct.node[i].data[4]);
			SerialUSB.print(" ");
			SerialUSB.print(msgStruct.node[i].data[5]);
			SerialUSB.print(" ");
			SerialUSB.print(msgStruct.node[i].data[6]);
			SerialUSB.print(" ");
			SerialUSB.println(msgStruct.node[i].data[7]);
			SerialUSB.println(msgStruct.node[i].isOn);
			SerialUSB.println(msgStruct.node[i].isDel);
			SerialUSB.println("");
			SerialUSB.println("");
			*/
		}
		myFile.close();
	}
}

// Reads users saved CAN Bus RX messages
void SDCard::readMAC(savedMACs& msgStruct)
{
	// File created and opened for writing
	File myFile = SD.open(F("SYSTEM/MAC.txt"), FILE_READ);

	if (myFile)
	{
		for (uint8_t i = 0; i < 10; i++)
		{
			char buffer[51];
			char s1[9];

			myFile.readBytesUntil('\n', buffer, 51);

			sscanf(buffer, "%s %x %x %x %x %x %x %x %x", 
				s1, &msgStruct.node[i].data[0], &msgStruct.node[i].data[1], &msgStruct.node[i].data[2], &msgStruct.node[i].data[3], &msgStruct.node[i].data[4], &msgStruct.node[i].data[5], &msgStruct.node[i].isOn, &msgStruct.node[i].isDel);

			if (!strcmp(s1, "(null)"))
			{
				char* temp = '\0';
				strncpy(msgStruct.node[i].name, "\0", 9);
			}
			else if (s1 == NULL)
			{
				strncpy(msgStruct.node[i].name, "\0", 9);
			}
			else if (s1 == "\0")
			{
				strncpy(msgStruct.node[i].name, "\0", 9);
			}
			else
			{
				strncpy(msgStruct.node[i].name, s1, 9);
			}

			/*
			SerialUSB.print("Index: ");
			SerialUSB.println(i);
			SerialUSB.println(msgStruct.node[i].name);
			SerialUSB.print("data: ");
			SerialUSB.print(msgStruct.node[i].data[0]);
			SerialUSB.print(" ");
			SerialUSB.print(msgStruct.node[i].data[1]);
			SerialUSB.print(" ");
			SerialUSB.print(msgStruct.node[i].data[2]);
			SerialUSB.print(" ");
			SerialUSB.print(msgStruct.node[i].data[3]);
			SerialUSB.print(" ");
			SerialUSB.print(msgStruct.node[i].data[4]);
			SerialUSB.print(" ");
			SerialUSB.println(msgStruct.node[i].data[5]);
			SerialUSB.println(msgStruct.node[i].isOn);
			SerialUSB.println(msgStruct.node[i].isDel);
			SerialUSB.println("");
			SerialUSB.println("");
			*/
		}
		myFile.close();
	}
}

// Reads in CAN Capture
void SDCard::readLogFile(char* filename)
{
	char tempStr[MSG_STRING_LENGTH];
	unsigned int messageNum = 0;
	unsigned int id = 0;
	unsigned int time = 0;
	unsigned int length = 0;
	unsigned int msg[8];

	File myFile = SD.open(filename, FILE_READ);

	//SerialUSB.println("");

	while (myFile.available())
	{
		myFile.readBytesUntil('\n', tempStr, MSG_STRING_LENGTH);
		sscanf(tempStr, "%d %d %x %d %x %x %x %x %x %x %x %x", &messageNum, &time, &id, &length,
			&msg[0], &msg[1], &msg[2], &msg[3], &msg[4], &msg[5], &msg[6], &msg[7]);
		SerialUSB.println(tempStr);
		/*
		SerialUSB.println(tempStr);
		SerialUSB.print("      ");
		SerialUSB.print(messageNum);
		SerialUSB.print("      ");
		SerialUSB.print(time);
		SerialUSB.print("      ");
		SerialUSB.print(id, HEX);
		SerialUSB.print("      ");
		SerialUSB.print(length);
		SerialUSB.print("  ");
		SerialUSB.print(msg[0], HEX);
		SerialUSB.print("  ");
		SerialUSB.print(msg[1], HEX);
		SerialUSB.print("  ");
		SerialUSB.print(msg[2], HEX);
		SerialUSB.print("  ");
		SerialUSB.print(msg[3], HEX);
		SerialUSB.print("  ");
		SerialUSB.print(msg[4], HEX);
		SerialUSB.print("  ");
		SerialUSB.print(msg[5], HEX);
		SerialUSB.print("  ");
		SerialUSB.print(msg[6], HEX);
		SerialUSB.print("  ");
		SerialUSB.println(msg[7], HEX);
		*/

		uint8_t data[8];
		data[0] = (uint8_t)msg[0];
		data[1] = (uint8_t)msg[1];
		data[2] = (uint8_t)msg[2];
		data[3] = (uint8_t)msg[3];
		data[4] = (uint8_t)msg[4];
		data[5] = (uint8_t)msg[5];
		data[6] = (uint8_t)msg[6];
		data[7] = (uint8_t)msg[7];

		can1.sendFrame(id, data, length, false);
		delay(5);

		if (Touch_getXY())
		{
			if ((y >= 275) && (y <= 315))
			{
				if ((x >= 131) && (x <= 216))
				{
					// Stop
					waitForItRect(131, 275, 216, 315);
					return;
				}
			}
		}
		backgroundProcess();
	}
	myFile.close();
}

// Reads in CAN Capture and returns true when end of file is reached
bool SDCard::readLogFileLCD(char* filename, uint32_t &index, bool isBackwards)
{
	#define MAX_LINES 17
	char tempStr[MSG_STRING_LENGTH];
	unsigned int messageNum = 0;
	unsigned int id = 0;
	unsigned int time = 0;
	unsigned int length = 0;
	unsigned int msg[8];
	uint16_t displayLineIndex = 60; // For printing to display
	bool endOfFile = false;

	// Backup the index to the starting index of the previous page
	if (isBackwards == true)
	{
		index = index -  (2 * (MAX_LINES * MSG_STRING_LENGTH)); // Subract two pages, the current page and the page to be printed
	}

	File myFile = SD.open(filename, FILE_READ);
	myFile.seek(index);
	myGLCD.setFont(SmallFont);

	if ((myFile.available()))
	{
		drawSquareBtn(131, 55, 479, 319, "", themeBackground, themeBackground, themeBackground, CENTER);
	}
	
	for (uint8_t i = 0; i < MAX_LINES; i++)
	{
		if ((myFile.available()))
		{
			myFile.readBytesUntil('\n', tempStr, MSG_STRING_LENGTH);

			sscanf(tempStr, "%d %d %x %d %x %x %x %x %x %x %x %x", &messageNum, &time, &id, &length,
				&msg[0], &msg[1], &msg[2], &msg[3], &msg[4], &msg[5], &msg[6], &msg[7]);

			myGLCD.setBackColor(VGA_WHITE);
			myGLCD.setColor(VGA_BLACK);

			char temp1[3];
			char temp2[2];

			myGLCD.printNumI(messageNum, 135, displayLineIndex);

			myGLCD.printNumI(time, 185, displayLineIndex);

			sprintf(temp1, "%03X", id);
			myGLCD.print(temp1, 240, displayLineIndex);

			myGLCD.printNumI(length, 270, displayLineIndex);

			sprintf(temp2, "%02X", msg[0]);
			myGLCD.print(temp2, 285, displayLineIndex);

			sprintf(temp2, "%02X", msg[1]);
			myGLCD.print(temp2, 310, displayLineIndex);

			sprintf(temp2, "%02X", msg[2]);
			myGLCD.print(temp2, 335, displayLineIndex);

			sprintf(temp2, "%02X", msg[3]);
			myGLCD.print(temp2, 360, displayLineIndex);

			sprintf(temp2, "%02X", msg[4]);
			myGLCD.print(temp2, 385, displayLineIndex);

			sprintf(temp2, "%02X", msg[5]);
			myGLCD.print(temp2, 410, displayLineIndex);

			sprintf(temp2, "%02X", msg[6]);
			myGLCD.print(temp2, 435, displayLineIndex);

			sprintf(temp2, "%02X", msg[7]);
			myGLCD.print(temp2, 460, displayLineIndex);

			displayLineIndex += 15;
		}
		else
		{
			// If the file becomes unavailable before the for loop finishes then the end was reached
			endOfFile = true;
		}
	}

	// Increment one page - Lines per page * length of line (string)
	index += (MAX_LINES * MSG_STRING_LENGTH);

	myFile.close();
	myGLCD.setFont(BigFont);
	return endOfFile;
}


/*=========================================================
	Create File Methods
===========================================================*/
// Create SD Card folder
void SDCard::createDRIVE(char* foldername)
{
	SD.mkdir(foldername);
}

// Reads in files
uint8_t SDCard::printDirectory(File dir, MyArray& list)
{
	const String str1 = "CANLOG";
	uint8_t count = 0;

	while (true)
	{
		File entry = dir.openNextFile();
		if (!entry || count > 9)
		{
			break;
		}
		if (entry.isDirectory() && !(str1.compareTo(entry.name())))
		{
			canDir = true;
		}
		else if (entry.isDirectory() && (str1.compareTo(entry.name())))
		{
			canDir = false;
		}
		if (canDir && !entry.isDirectory())
		{
			sprintf(list[count], "%s", entry.name());
			count++;
		}
		if (entry.isDirectory()) {
			printDirectory(entry, list);
		}
		entry.close();
	}
	return count;
}

// Size of canlog file in bytes
uint32_t SDCard::fileSize(char* filename)
{
	char temp[40];
	strcpy(temp, "canlog/");
	strcat(temp, filename);
	File myFile = SD.open(temp, FILE_READ);
	uint32_t size = myFile.size();
	return size;
}

// Does what is says
uint32_t SDCard::fileLength(char* filename)
{
	String inString;
	uint32_t length = 0;
	File myFile = SD.open(filename, FILE_READ);

	while (myFile.available())
	{
		inString = myFile.readStringUntil('\n');
		length++;
	}

	myFile.close();
	return length;
}

// Splits file into two halves
void SDCard::split(char* filename, uint32_t size)
{
	uint32_t splitFile;
	String inString;
	uint32_t count = 0;

	// Split and account for missing rounded number if odd
	(size % 2 > 0) ? splitFile = (size / 2) + 1 : splitFile = size / 2;

	deleteFile("canlog/a.txt");
	deleteFile("canlog/b.txt");
	File myFileW1 = SD.open("canlog/a.txt", FILE_WRITE);
	File myFileW2 = SD.open("canlog/b.txt", FILE_WRITE);
	File myFile = SD.open(filename, FILE_READ);

	// TODO FIX
	while (myFile.available())
	{
		inString = myFile.readStringUntil('\n');

		for (uint8_t i = 64; i < 70; i++)
		{
			if (inString[i] == '\r')
			{
				inString.remove(i);
				break;
			}
		}
		if (count < splitFile)
		{
			myFileW1.print(inString + '\n');
		}
		else
		{
			myFileW2.print(inString + '\n');
		}
		count++;
	}
	
	myFile.close();
	myFileW1.close();
	myFileW2.close();
}

// Makes a temporary copy 
void SDCard::tempCopy(char* filename)
{
	String inString;

	File myFile = SD.open(filename, FILE_READ);
	File myFileW1 = SD.open("canlog/temp.txt", FILE_WRITE);

	while (myFile.available())
	{
		inString = myFile.readStringUntil('\n');
		myFileW1.println(inString);
	}
	myFile.close();
	myFileW1.close();
}