// SDCard class manages the SD card reader hardware

#include "SDCard.h"

File myFile;
CANBus can;

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
    return;
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
    return;
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
    return;
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

// Reads in CAN Capture
void SDCard::readLogFile(char * filename)
{
    //char fileLoc[20] = "CANLOG/";
    //strcat(fileLoc, filename);
    // File created and opened for writing
    //SerialUSB.println(filename);
    myFile = SD.open(filename, FILE_READ);
    char tempStr[64];
    int messageNum, id;
    float time;
    uint8_t length = 0;
    byte msg[8];
    bool sendIt = true;

    // readBytesUntil is reading an empty line after each line and I dont know why
    // sendIt prevents sending the same message twice after the empty line
    while (myFile.available())
    {   
        myFile.readBytesUntil('\n', tempStr, 64);
        sscanf(tempStr, "%d %f %x %d %x %x %x %x %x %x %x %x", &messageNum, &time, &id, &length, &msg[0], &msg[1], &msg[2], &msg[3], &msg[4], &msg[5], &msg[6], &msg[7]);
        /*
        SerialUSB.println(tempStr);
        SerialUSB.print("ID: ");
        SerialUSB.print(id);
        SerialUSB.print("  Length: ");
        SerialUSB.print(length);
        SerialUSB.print("  MSG: ");
        SerialUSB.println(msg[0]);
        */
        if (sendIt)
        {
            can.sendFrame(id, msg, length, false);
            delay(8);
        }
        sendIt = !sendIt;
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
uint8_t SDCard::printDirectory(File dir, MyArray &list)
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

/*
void SDCard::split(char* filename, uint32_t size)
{
    SerialUSB.println("");
    SerialUSB.println(filename);
    SerialUSB.println(size);

    uint32_t fileSize;
    String tempStr;
    uint32_t count = 0;

    (size % 2 > 0) ? fileSize = (size / 2) + 1 : fileSize = size / 2;

    //SerialUSB.println(fileSize);

    myFile = SD.open(filename, FILE_READ);
    File myFileW1 = SD.open("canlog/temp.txt", FILE_WRITE);

    //SerialUSB.println("Temp");
    while (myFile.available())
    {
        tempStr = myFile.readStringUntil('\n');
        myFileW1.print(tempStr);
    }
    myFile.close();
    myFileW1.close();


    File myFile2 = SD.open("canlog/temp.txt", FILE_READ);
    File myFileW2 = SD.open("canlog/a.txt", FILE_WRITE);
    File myFileW3 = SD.open("canlog/b.txt", FILE_WRITE);

    //SerialUSB.println("A");
    while (myFile2.available())
    {
        tempStr = myFile2.readStringUntil('\n');
        
        if (count < fileSize)
        {
            myFileW2.print(tempStr);
            //SerialUSB.print("a: ");
            //SerialUSB.println(count);
        }
        else if (count == fileSize)
        {
            //SerialUSB.println("B");
            myFileW2.close();
            
            myFileW3.print(tempStr);
        }
        else if ( count > fileSize)
        {
            myFileW3.print(tempStr);
            //SerialUSB.print("b: ");
            //SerialUSB.println(count);
        }
        


        count++;
    }
    myFile2.close();
    myFileW3.close();
}
*/