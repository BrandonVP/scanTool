// SDCard class manages the SD card reader hardware

#include "SDCard.h"

#define SD_CARD_CS 8
#define ROW_DIM 100

// File object
File myFile;

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


void SDCard::readFile(char* filename, uint8_t* arrayIn)
{
    Serial.println("in");
    // File created and opened for writing
    myFile = SD.open(filename, FILE_READ);
    int i = 0;
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


/*=========================================================
    Create File Methods
===========================================================*/
// Create SD Card folder
void SDCard::createDRIVE(char* foldername)
{
    SD.mkdir(foldername);
}


