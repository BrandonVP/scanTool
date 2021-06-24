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

void SDCard::readLogFile()
{
    Serial.println("in");
    // File created and opened for writing
    myFile = SD.open("readcan.txt", FILE_READ);
    int i = 0;
    uint16_t ID = 0;
    uint8_t data[8];
    String tempStr;
    char c[20];
    while (myFile.available())
    {
        tempStr = (myFile.readStringUntil(' '));
        strcpy(c, tempStr.c_str());
        if (i == 1)
        {
            ID = strtol(c, NULL, 16);
            //Serial.print(ID, HEX);
            //Serial.print(" ");
        }
        if (i > 2 && i < 11)
        {
            data[i - 3] = strtol(c, NULL, 16);
            //Serial.print(data[i - 3], HEX);
            //Serial.print(" ");
        }

        i++;
        if (i == 11)
        {
            i = 0;
            //Serial.println(" ");
            can.sendFrame(ID, data);
            delay(20);
        }
    }
    
    myFile.close();
}

void SDCard::readLogFile2()
{
    Serial.println("in");
    // File created and opened for writing
    myFile = SD.open("readcan2.txt", FILE_READ);
    int i = 0;
    uint16_t ID = 0;
    uint8_t data[8];
    String tempStr;
    char c[20];
    while (myFile.available())
    {
        tempStr = (myFile.readStringUntil(' '));
        strcpy(c, tempStr.c_str());
        ID = strtol(c, NULL, 16);
        Serial.println(ID, HEX);
        /*
        if (i == 1)
        {
            ID = strtol(c, NULL, 16);
            //Serial.print(ID, HEX);
            //Serial.print(" ");
        }
        if (i > 2 && i < 11)
        {
            data[i - 3] = strtol(c, NULL, 16);
            //Serial.print(data[i - 3], HEX);
            //Serial.print(" ");
        }

        i++;
        if (i == 11)
        {
            i = 0;
            //Serial.println(" ");
            can.sendFrame(ID, data);
            delay(20);
        }
        */
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


