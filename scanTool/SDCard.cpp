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


/*=========================================================
    Read File Methods
===========================================================*/
// size_t is a type able to represent the size of any object in bytes
size_t SDCard::readField(File* file, char* str, size_t size, const char* delim) {
    char ch;
    size_t n = 0;
    while ((n + 1) < size && file->read(&ch, 1) == 1) {
        // Delete CR.
        //if (ch == '\r') {
        //    continue;
        //}
        str[n++] = ch;
        if (strchr(delim, ch)) {
            break;
        }
    }
    str[n] = '\0';
    return n;
}

// Modified SdFat library code to read field in text file from sd
uint8_t* SDCard::readFile(char* filename, uint8_t* arrayIn)
{
    // File created and opened for writing
    myFile = SD.open(filename, FILE_READ);

    // Check if file was sucsefully open
    if (myFile)
    {
        //myFile.rewind();
        // Array for data.

        int i = 0;     // First array index.
        size_t n;      // Length of returned field with delimiter.
        char str[10];  // Must hold longest field with delimiter and zero byte.
        char* ptr;     // Test for valid field.

        // Read the file and store the data.
        int* fileSizePtr;
        for (i = 0; i < ROW_DIM; i++) {
            n = readField(&myFile, str, sizeof(str), "x");
            arrayIn[i] = strtol(str, &ptr, 16);
            //Serial.println(array[i], HEX);
            while (*ptr == ' ') {
                ptr++;
            }
            // Allow missing endl at eof.
            //if (str[n - 1] != '\n' && file.available()) {
            //    errorHalt("missing endl");
            //}
        }
        //arrayIn* sdPtr = &array;
        // Print the array.
        //for (i = 0; i < ROW_DIM; i++) {
        //        //sdArray[i][j] = array[i][j];
        //        Serial.println(array[i], HEX);
        //}
        myFile.close();
    }
    else
    {
        Serial.println(F("Unable to open file"));
        //return;
    }
    return arrayIn;
}


/*=========================================================
    Create File Methods
===========================================================*/
// Create SD Card folder
void SDCard::createDRIVE(char* foldername)
{
    SD.mkdir(foldername);
}


