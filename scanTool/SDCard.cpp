// SDCard class manages the SD card reader hardware

//#include <SD.h>
#include "SDCard.h"

#define SD_CARD_CS 8
#define ROW_DIM 9
#define COL_DIM 8
typedef uint8_t arrayIn[ROW_DIM][COL_DIM];

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

/*
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
*/

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

void SDCard::deleteFile(char* filename)
{
    //remove any existing file with this name
    SD.remove(filename); 
}

// size_t is a type able to represent the size of any object in bytes
size_t SDCard::readField(File* file, char* str, size_t size, const char* delim) {
    char ch;
    size_t n = 0;
    while ((n + 1) < size && file->read(&ch, 1) == 1) {
        // Delete CR.
        if (ch == '\r') {
            continue;
        }
        str[n++] = ch;
        if (strchr(delim, ch)) {
            break;
        }
    }
    str[n] = '\0';
    return n;
}

void SDCard::createDRIVE(char* foldername)
{
    SD.mkdir(foldername);
    Serial.println(foldername);
}


// Modified SdFat library code to read field in text file from sd
void SDCard::readFile(char* filename)
{
    // File created and opened for writing
    myFile = SD.open("PIDSCAN.txt", FILE_READ);

    // Check if file was sucsefully open
    if (myFile)
    {
        //myFile.rewind();
        // Array for data.
        arrayIn array;
        int i = 0;     // First array index.
        int j = 0;     // Second array index
        size_t n;      // Length of returned field with delimiter.
        char str[20];  // Must hold longest field with delimiter and zero byte.
        char* ptr;     // Test for valid field.
        // Read the file and store the data.
        int* fileSizePtr;
        for (i = 0; i < ROW_DIM; i++) {
            for (j = 0; j < COL_DIM; j++) {
                n = readField(&myFile, str, sizeof(str), " \n");
                array[i][j] = strtol(str, &ptr, 16);
                while (*ptr == ' ') {
                    ptr++;
                }
            }
            // Allow missing endl at eof.
            //if (str[n - 1] != '\n' && file.available()) {
            //    errorHalt("missing endl");
            //}
        }
        arrayIn* sdPtr = &array;
        // Print the array.
        for (i = 0; i < ROW_DIM; i++) {
            for (j = 0; j < COL_DIM; j++) {
                if (j) {
                    //Serial.print(' ');
                }
                //sdArray[i][j] = array[i][j];
                Serial.print(array[i][j], HEX);
            }
            Serial.println();
        }
        myFile.close();
    }
}


/*
// Code to read field taken from SdFat library
// size_t is a type able to represent the size of any object in bytes
size_t readField(File* file, char* str, size_t size, const char* delim) {
    char ch;
    size_t n = 0;
    while ((n + 1) < size && file->read(&ch, 1) == 1) {
        // Delete CR.
        if (ch == '\r') {
            continue;
        }
        str[n++] = ch;
        if (strchr(delim, ch)) {
            break;
        }
    }
    str[n] = '\0';
    return n;
}
// Modified SdFat library code to read field in text file from sd
void readInSD()
{
file.rewind();
// Array for data.
arrayIn array;
int i = 0;     // First array index.
int j = 0;     // Second array index
size_t n;      // Length of returned field with delimiter.
char str[20];  // Must hold longest field with delimiter and zero byte.
char* ptr;     // Test for valid field.
// Read the file and store the data.
int* fileSizePtr;
for (i = 0; i < ROW_DIM; i++) {
    for (j = 0; j < COL_DIM; j++) {
        n = readField(&file, str, sizeof(str), ",\n");
        array[i][j] = strtol(str, &ptr, 16);
        while (*ptr == ' ') {
            ptr++;
        }
    }
    // Allow missing endl at eof.
    //if (str[n - 1] != '\n' && file.available()) {
    //    errorHalt("missing endl");
    //}
}
arrayIn* sdPtr = &array;
// Print the array.
for (i = 0; i < ROW_DIM; i++) {
    for (j = 0; j < COL_DIM; j++) {
        if (j) {
            //Serial.print(' ');
        }
        sdArray[i][j] = array[i][j];
        Serial.print(array[i][j], HEX);
    }
    Serial.println();
}
file.close();
}
*/