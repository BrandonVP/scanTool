// SDCard class manages the SD card reader hardware

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
	myFile = SD.open(filename, FILE_WRITE);

	// Check if file was sucsefully open
	if (myFile)
	{
		myFile.print(incoming);
		myFile.close();
	}
}

void SDCard::setSDFilename(char* filename)
{
	for (uint8_t i = 0; i < 50; i++)
	{
		SDfilename[i] = '\0';
	}
	char* a1 = SDfilename;
	memcpy(a1, "canlog/", 7);
	strcat(SDfilename, filename);
	SerialUSB.println(SDfilename);
}

// Write string to SD Card
void SDCard::writeFileS(uint8_t* incoming)
{
	// File created and opened for writing
	myFile = SD.open(SDfilename, FILE_WRITE);

	// Check if file was sucsefully open
	if (myFile)
	{
		myFile.write(incoming, SD_CAPTURE_BLOCK_SIZE);
		myFile.close();
	}
}

// Write integer and base to SD Card
void SDCard::writeFile(char* filename, int incoming, int base)
{
	// File created and opened for writing
	myFile = SD.open(filename, FILE_WRITE);

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
	myFile = SD.open(filename, FILE_WRITE);

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
	myFile = SD.open("SYSTEM/CANMsg.txt", FILE_WRITE);

	for (uint8_t i = 0; i < 20; i++)
	{
		if (strcmp(msgStruct.node[i].name, "\0"))
		{
			sprintf(buffer, "%s %x %x %x %x %x %x %x %x %x %x %x %x %x \n", msgStruct.node[i].name, msgStruct.node[i].channel, msgStruct.node[i].interval, msgStruct.node[i].id,
				msgStruct.node[i].data[0], msgStruct.node[i].data[1], msgStruct.node[i].data[2], msgStruct.node[i].data[3], msgStruct.node[i].data[4], msgStruct.node[i].data[5], msgStruct.node[i].data[6], msgStruct.node[i].data[7],
				msgStruct.node[i].isOn, msgStruct.node[i].isDel);
			SerialUSB.println(msgStruct.node[i].name);
		}
		else
		{
			sprintf(buffer, "%s %x %x %x %x %x %x %x %x %x %x %x %x %x \n", "(null)", msgStruct.node[i].channel, msgStruct.node[i].interval, msgStruct.node[i].id,
				msgStruct.node[i].data[0], msgStruct.node[i].data[1], msgStruct.node[i].data[2], msgStruct.node[i].data[3], msgStruct.node[i].data[4], msgStruct.node[i].data[5], msgStruct.node[i].data[6], msgStruct.node[i].data[7],
				msgStruct.node[i].isOn, msgStruct.node[i].isDel);
			SerialUSB.println("(null)");
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
	myFile = SD.open(filename, FILE_READ);
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
	myFile = SD.open("SYSTEM/CANMsg.txt", FILE_READ);

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

// Reads in CAN Capture
void SDCard::readLogFile(char* filename)
{
	myFile = SD.open(filename, FILE_READ);
	char tempStr[MSG_STRING_LENGTH];
	int messageNum, id;
	float time;
	uint8_t length = 0;
	byte msg[8];

	while (myFile.available())
	{
		myFile.readBytesUntil('\n', tempStr, MSG_STRING_LENGTH);
		sscanf(tempStr, "%d %f %x %d %x %x %x %x %x %x %x %x", &messageNum, &time, &id, &length, &msg[0], &msg[1], &msg[2], &msg[3], &msg[4], &msg[5], &msg[6], &msg[7]);
		
		/*
		SerialUSB.print(tempStr);
		SerialUSB.print(" ");
		SerialUSB.print(messageNum);
		SerialUSB.print(" ");
		SerialUSB.print(time);
		SerialUSB.print(" ");
		SerialUSB.print(id);
		SerialUSB.print(" ");
		SerialUSB.println(length);
		SerialUSB.print(" ");
		SerialUSB.print(msg[0]);
		SerialUSB.println("");
		*/
		
		can1.sendFrame(id, msg, length, false);
		delay(6);

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
	}
	myFile.close();
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

uint32_t SDCard::fileLength(char* filename)
{
	char tempStr[64];
	uint32_t length = 0;

	myFile = SD.open(filename, FILE_READ);

	while (myFile.available())
	{
		myFile.readBytesUntil('\n', tempStr, 64);
		length++;
	}
	myFile.close();
	return length / 2;
}

void SDCard::split(char* filename, uint32_t size)
{
	//SerialUSB.println(filename);

	uint32_t fileSize;
	String tempStr;
	//char tempStr[64];
	uint32_t count = 0;

	//SerialUSB.println(size);
	(size % 2 > 0) ? fileSize = (size / 2) + 1 : fileSize = size / 2;
	//SerialUSB.println(fileSize);

	myFile = SD.open(filename, FILE_READ);
	deleteFile("canlog/a.txt");
	deleteFile("canlog/b.txt");

	File myFileW1 = SD.open("canlog/a.txt", FILE_WRITE);
	File myFileW2 = SD.open("canlog/b.txt", FILE_WRITE);

	while (myFile.available())
	{
		tempStr = myFile.readStringUntil('\n');
		//myFile.readBytesUntil('\n', tempStr, 64);

		if (count < fileSize)
		{
			myFileW1.println(tempStr);
			//SerialUSB.print("a: ");
			//SerialUSB.println(count);
		}
		else
		{
			myFileW2.println(tempStr);
			//SerialUSB.print("b: ");
			//SerialUSB.println(count);
		}
		count++;
	}
	myFile.close();
	myFileW1.close();
	myFileW2.close();
}

void SDCard::tempCopy(char* filename)
{
	//SerialUSB.println("here");
	String tempStr;
	myFile = SD.open(filename, FILE_READ);
	File myFileW1 = SD.open("canlog/temp.txt", FILE_WRITE);
	//SerialUSB.println(filename);
	while (myFile.available())
	{
		tempStr = myFile.readStringUntil('\n');
		//SerialUSB.println(tempStr);
		myFileW1.println(tempStr);
	}
	myFile.close();
	myFileW1.close();
	//SerialUSB.println("Leaving");
}