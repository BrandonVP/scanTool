/*
 Name:    scanTool.ino
 Created: 11/15/2020 8:27:18 AM
 Author:  Brandon Van Pelt
*/

/*=========================================================
    Todo List
===========================================================
Read DTCs
Send custom message
-Export functions to free up main
-
CLEAN up globals!!! (too many)
===========================================================
    End Todo List
=========================================================*/

#include <UTFT.h>
#include <SD.h>
#include <UTouchCD.h>
#include <memorysaver.h>
#include <SPI.h>
#include <UTouch.h>
#include "CANBus.h"
#include "definitions.h"
#include "icons.h"
#include "SDCard.h"

// Initialize display
//(byte model, int RS, int WR, int CS, int RST, int SER)
UTFT myGLCD(ILI9488_16, 7, 38, 9, 10);
//RTP: byte tclk, byte tcs, byte din, byte dout, byte irq
UTouch  myTouch(2, 6, 3, 4, 5);
uint8_t brightnessLevel = 0;

// For touch controls
int x, y;

// Used for page control
uint8_t controlPage = 0;
uint8_t page = 0;
bool hasDrawn = false;

// External import for fonts
extern uint8_t SmallFont[];
extern uint8_t BigFont[];

// Object to control CAN Bus hardware
CANBus can1;

// Object to control SD Card Hardware
SDCard sdCard;

// BITMAP global for bmpDraw()
int dispx, dispy;

// Used for PID scroll function
char currentDir[20];

uint8_t arrayIn[100];

// Performing a sucessful PID scan will change this to true
bool hasPID = false;

// Keeps track of current line when displaying CAN traffic on LCD
uint16_t indexCANMsg = 60;

// Filter range at startup
uint32_t startRange = 0x000;
uint32_t endRange = 0xFFF;

int pageControl();


/*=========================================================
    Framework Functions
===========================================================*/
//
void bmpDraw(char* filename, int x, int y) {
    File     bmpFile;
    int      bmpWidth, bmpHeight;   // W+H in pixels
    uint8_t  bmpDepth;              // Bit depth (currently must be 24)
    uint32_t bmpImageoffset;        // Start of image data in file
    uint32_t rowSize;               // Not always = bmpWidth; may have padding
    uint8_t  sdbuffer[3 * BUFFPIXEL]; // pixel in buffer (R+G+B per pixel)
    uint16_t lcdbuffer[BUFFPIXEL];  // pixel out buffer (16-bit per pixel)
    uint8_t  buffidx = sizeof(sdbuffer); // Current position in sdbuffer
    boolean  goodBmp = false;       // Set to true on valid header parse
    boolean  flip = true;        // BMP is stored bottom-to-top
    int      w, h, row, col;
    uint8_t  r, g, b;
    uint32_t pos = 0, startTime = millis();
    uint8_t  lcdidx = 0;
    boolean  first = true;

    if ((x >= dispx) || (y >= dispy)) return;

    Serial.println();
    Serial.print(F("Loading image '"));
    Serial.print(filename);
    Serial.println('\'');

    // Open requested file on SD card
    if ((bmpFile = SD.open(filename)) == NULL) {
        Serial.println(F("File not found"));
        return;
    }


    // Parse BMP header
    if (read16(bmpFile) == 0x4D42) { // BMP signature

        Serial.println(read32(bmpFile));
        (void)read32(bmpFile); // Read & ignore creator bytes
        bmpImageoffset = read32(bmpFile); // Start of image data
        Serial.print(F("Image Offset: "));
        Serial.println(bmpImageoffset, DEC);

        // Read DIB header
        Serial.print(F("Header size: "));
        Serial.println(read32(bmpFile));
        bmpWidth = read32(bmpFile);
        bmpHeight = read32(bmpFile);

        if (read16(bmpFile) == 1) { // # planes -- must be '1'
            bmpDepth = read16(bmpFile); // bits per pixel
            Serial.print(F("Bit Depth: "));
            Serial.println(bmpDepth);
            if ((bmpDepth == 24) && (read32(bmpFile) == 0)) { // 0 = uncompressed
                goodBmp = true; // Supported BMP format -- proceed!
                Serial.print(F("Image size: "));
                Serial.print(bmpWidth);
                Serial.print('x');
                Serial.println(bmpHeight);

                // BMP rows are padded (if needed) to 4-byte boundary
                rowSize = (bmpWidth * 3 + 3) & ~3;

                // If bmpHeight is negative, image is in top-down order.
                // This is not canon but has been observed in the wild.
                if (bmpHeight < 0) {
                    bmpHeight = -bmpHeight;
                    flip = false;
                }

                // Crop area to be loaded
                w = bmpWidth;
                h = bmpHeight;
                if ((x + w - 1) >= dispx)  w = dispx - x;
                if ((y + h - 1) >= dispy) h = dispy - y;

                // Set TFT address window to clipped image bounds

                for (row = 0; row < h; row++) { // For each scanline...
                  // Seek to start of scan line.  It might seem labor-
                  // intensive to be doing this on every line, but this
                  // method covers a lot of gritty details like cropping
                  // and scanline padding.  Also, the seek only takes
                  // place if the file position actually needs to change
                  // (avoids a lot of cluster math in SD library).
                    if (flip) // Bitmap is stored bottom-to-top order (normal BMP)
                        pos = bmpImageoffset + (bmpHeight - 1 - row) * rowSize;
                    else     // Bitmap is stored top-to-bottom
                        pos = bmpImageoffset + row * rowSize;
                    if (bmpFile.position() != pos) { // Need seek?
                        bmpFile.seek(pos);
                        buffidx = sizeof(sdbuffer); // Force buffer reload
                    }

                    for (col = 0; col < w; col++) { // For each column...
                      // Time to read more pixel data?
                        if (buffidx >= sizeof(sdbuffer)) { // Indeed
                          // Push LCD buffer to the display first
                            if (lcdidx > 0) {
                                myGLCD.setColor(lcdbuffer[lcdidx]);
                                myGLCD.drawPixel(col + 155, row + 1);
                                lcdidx = 0;
                                first = false;
                            }

                            bmpFile.read(sdbuffer, sizeof(sdbuffer));
                            buffidx = 0; // Set index to beginning
                        }

                        // Convert pixel from BMP to TFT format
                        b = sdbuffer[buffidx++];
                        g = sdbuffer[buffidx++];
                        r = sdbuffer[buffidx++];
                        myGLCD.setColor(r, g, b);
                        myGLCD.drawPixel(col + 155, row + 1);
                    } // end pixel

                } // end scanline

                // Write any remaining data to LCD
                if (lcdidx > 0) {
                    myGLCD.setColor(lcdbuffer[lcdidx]);
                    myGLCD.drawPixel(col + 155, row + 1);
                }
            } // end goodBmp
        }
    }

    bmpFile.close();
    if (!goodBmp) Serial.println(F("BMP format not recognized."));

}

// These read 16- and 32-bit types from the SD card file.
// BMP data is stored little-endian, Arduino is little-endian too.
// May need to reverse subscript order if porting elsewhere.
uint16_t read16(File f) {
    uint16_t result;
    ((uint8_t*)&result)[0] = f.read(); // LSB
    ((uint8_t*)&result)[1] = f.read(); // MSB
    return result;
}
uint32_t read32(File f) {
    uint32_t result;
    ((uint8_t*)&result)[0] = f.read(); // LSB
    ((uint8_t*)&result)[1] = f.read();
    ((uint8_t*)&result)[2] = f.read();
    ((uint8_t*)&result)[3] = f.read(); // MSB
    return result;
}

// Custom bitmap
void print_icon(int x, int y, const unsigned char icon[]) {
    myGLCD.setColor(menuBtnColor);
    myGLCD.setBackColor(themeBackground);
    int i = 0, row, column, bit, temp;
    int constant = 1;
    for (row = 0; row < 40; row++) {
        for (column = 0; column < 5; column++) {
            temp = icon[i];
            for (bit = 7; bit >= 0; bit--) {

                if (temp & constant) {
                    myGLCD.drawPixel(x + (column * 8) + (8 - bit), y + row);
                }
                else {

                }
                temp >>= 1;
            }
            i++;
        }
    }
}

/*****************************************************
*  Draw Round/Square Button                          *
*                                                    *
*  Description:   Draws shapes with/without text     *
*                                                    *
*  Parameters: int: x start, y start, x stop, y stop *
*              String: Button text                   *
*              Hex value: Background Color           *
*              Hex value: Border of shape            *
*              Hex value: Color of text              *
*              int: Alignment of text #defined as    *
*                   LEFT, CENTER, RIGHT              *
*                                                    *
*****************************************************/
void drawRoundBtn(int x_start, int y_start, int x_stop, int y_stop, String button, int backgroundColor, int btnBorderColor, int btnTxtColor, int align) {
    int size, temp, offset;

    myGLCD.setColor(backgroundColor);
    myGLCD.fillRoundRect(x_start, y_start, x_stop, y_stop); // H_Start, V_Start, H_Stop, V_Stop
    myGLCD.setColor(btnBorderColor);
    myGLCD.drawRoundRect(x_start, y_start, x_stop, y_stop);
    myGLCD.setColor(btnTxtColor); // text color
    myGLCD.setBackColor(backgroundColor); // text background
    switch (align)
    {
    case 1:
        myGLCD.print(button, x_start + 5, y_start + ((y_stop - y_start) / 2) - 8); // hor, ver
        break;
    case 2:
        size = button.length();
        temp = ((x_stop - x_start) / 2);
        offset = x_start + (temp - (8 * size));
        myGLCD.print(button, offset, y_start + ((y_stop - y_start) / 2) - 8); // hor, ver
        break;
    case 3:
        // Currently hotwired for deg text only
        myGLCD.print(button, x_start + 55, y_start + ((y_stop - y_start) / 2) - 8); // hor, ver
        break;
    default:
        break;
    }

}
void drawSquareBtn(int x_start, int y_start, int x_stop, int y_stop, String button, int backgroundColor, int btnBorderColor, int btnTxtColor, int align) {
    int size, temp, offset;
    myGLCD.setColor(backgroundColor);
    myGLCD.fillRect(x_start, y_start, x_stop, y_stop); // H_Start, V_Start, H_Stop, V_Stop
    myGLCD.setColor(btnBorderColor);
    myGLCD.drawRect(x_start, y_start, x_stop, y_stop);
    myGLCD.setColor(btnTxtColor); // text color
    myGLCD.setBackColor(backgroundColor); // text background
    switch (align)
    {
    case 1:
        myGLCD.print(button, x_start + 5, y_start + ((y_stop - y_start) / 2) - 8); // hor, ver
        break;
    case 2:
        size = button.length();
        temp = ((x_stop - x_start) / 2);
        offset = x_start + (temp - (8 * size));
        myGLCD.print(button, offset, y_start + ((y_stop - y_start) / 2) - 8); // hor, ver
        break;
    case 3:
        //align left
        break;
    default:
        break;
    }
}

// Function complete load bar
void loadBar(int progress)
{
    if (progress >= DONE)
    {
        drawSquareBtn(200, 200, 400, 220, "Finished", menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
        //myGLCD.setColor(menuBtnColor);
        //myGLCD.fillRect(200, 200, 400, 220);

        return;
    }
    drawSquareBtn(199, 199, 401, 221, "", themeBackground, menuBtnBorder, menuBtnText, LEFT);
    myGLCD.setColor(menuBtnColor);
    myGLCD.fillRect(200, 200, (200 + (progress * 25)), 220);
}

// Function to highlight buttons when clicked
void waitForIt(int x1, int y1, int x2, int y2)
{
    myGLCD.setColor(themeBackground);
    myGLCD.drawRoundRect(x1, y1, x2, y2);
    while (myTouch.dataAvailable())
        myTouch.read();
    myGLCD.setColor(menuBtnBorder);
    myGLCD.drawRoundRect(x1, y1, x2, y2);
}
void waitForItRect(int x1, int y1, int x2, int y2)
{
    myGLCD.setColor(themeBackground);
    myGLCD.drawRect(x1, y1, x2, y2);
    while (myTouch.dataAvailable())
        myTouch.read();
    myGLCD.setColor(menuBtnBorder);
    myGLCD.drawRect(x1, y1, x2, y2);
}


/*=========================================================
    CAN Bus
===========================================================*/
void drawCANBus()
{
    drawSquareBtn(145, 60, 479, 319, "", themeBackground, themeBackground, themeBackground, CENTER);
    drawRoundBtn(145, 80, 308, 130, "CAN0: LCD", menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
    drawRoundBtn(312, 80, 475, 130, "Serial", menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
    drawRoundBtn(145, 135, 308, 185, "CAN1: LCD", menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
    drawRoundBtn(312, 135, 475, 185, "Serial", menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
    //drawRoundBtn(145, 190, 308, 240, "Unused", menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
    //drawRoundBtn(312, 190, 475, 240, "Unused", menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
    //drawRoundBtn(145, 245, 308, 295, "Unused", menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
    //drawRoundBtn(312, 245, 475, 295, "Unused", menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
    drawSquareBtn(150, 300, 479, 319, VERSION, themeBackground, themeBackground, menuBtnColor, CENTER);
}

void CANBusButtons()
{
    // Touch screen controls
    if (myTouch.dataAvailable())
    {
        myTouch.read();
        x = myTouch.getX();
        y = myTouch.getY();

        // Start Scan
        if ((x >= 145) && (x <= 308))
        {
            if ((y >= 80) && (y <= 130))
            {
                waitForIt(145, 80, 308, 130);
                // CAN0: LCD
                page = 1;
                hasDrawn = false;
            }
            if ((y >= 135) && (y <= 185))
            {
                waitForIt(145, 135, 308, 185);
                // CAN1: LCD
                page = 3;
                hasDrawn = false;
            }
            if ((y >= 190) && (y <= 240))
            {
                waitForIt(145, 190, 308, 240);
                // Unused
                //page = 5;
                //hasDrawn = false;
            }
            if ((y >= 245) && (y <= 295))
            {
                waitForIt(145, 245, 308, 295);
                // Unused
                //page = 7;
                //hasDrawn = false;
            }
        }
        if ((x >= 312) && (x <= 475))
        {
            if ((y >= 80) && (y <= 130))
            {
                waitForIt(312, 80, 475, 130);
                // Unused
                page = 2;
                hasDrawn = false;
            }
            if ((y >= 135) && (y <= 185))
            {
                waitForIt(312, 135, 475, 185);
                // CAN1 Serial
                page = 4;
                hasDrawn = false;

            }
            if ((y >= 190) && (y <= 240))
            {
                waitForIt(312, 190, 475, 240);
                // Unused
                //page = 6;
                hasDrawn = false;
            }
            if ((y >= 245) && (y <= 295))
            {
                waitForIt(312, 245, 475, 295);
                // Unused
                //page = 8;
                //hasDrawn = false;
            }
        }
    }
}

/*============== CAN: LCD ==============*/
void drawReadInCANLCD()
{
    drawSquareBtn(145, 60, 479, 319, "", themeBackground, themeBackground, themeBackground, CENTER);
}

void readInCANMsg(uint8_t channel)
{
    myGLCD.setBackColor(VGA_WHITE);
    myGLCD.setFont(SmallFont);
    uint8_t rxBuf[8];
    uint32_t rxId;
    if (can1.getMessage(rxBuf, rxId, channel))
    {
        char printString[50];
        myGLCD.setColor(VGA_WHITE);
        myGLCD.fillRect(150, (indexCANMsg - 5), 479, (indexCANMsg + 25));
        myGLCD.setColor(VGA_BLACK);
        sprintf(printString, "ID: %X D: %X %X %X %X %X %X %X %X", rxId, rxBuf[0], rxBuf[1], rxBuf[2], rxBuf[3], rxBuf[4], rxBuf[5], rxBuf[6], rxBuf[7]);
        myGLCD.print(printString, 150, indexCANMsg);

        if (indexCANMsg < 300)
        {
            indexCANMsg += 15;
        }
        else
        {
            indexCANMsg = 60;
        }
    }
    myGLCD.setFont(BigFont);
}


/*============== CAN: Serial ==============*/
void drawCANSerial()
{
    drawSquareBtn(145, 60, 479, 319, "", themeBackground, themeBackground, themeBackground, CENTER);
    drawSquareBtn(145, 140, 479, 160, "View CAN on serial", themeBackground, themeBackground, menuBtnColor, CENTER);
}

/*============== Send Frame CAN1 ==============*/
// Draw
// Function
// Buttons

/*============== Send Frame CAN2 ==============*/
// Draw
// Function
// Buttons


/*=========================================================
    Vehicle Tools
===========================================================*/
void drawVehicleTools()
{
    drawSquareBtn(145, 60, 479, 319, "", themeBackground, themeBackground, themeBackground, CENTER);
    drawRoundBtn(145, 80, 308, 130, "PIDSCAN", menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
    drawRoundBtn(312, 80, 475, 130, "PIDSTRM", menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
    drawRoundBtn(145, 135, 308, 185, "PID Guages", menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
    drawRoundBtn(312, 135, 475, 185, "VIN", menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
    drawRoundBtn(145, 190, 308, 240, "DTC Scan", menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
    drawRoundBtn(312, 190, 475, 240, "DTC Clear", menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
    //drawRoundBtn(145, 245, 308, 295, "Unused", menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
    //drawRoundBtn(312, 245, 475, 295, "Unused", menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
    drawSquareBtn(150, 300, 479, 319, VERSION, themeBackground, themeBackground, menuBtnColor, CENTER);
}

void VehicleToolButtons()
{
    // Touch screen controls
    if (myTouch.dataAvailable())
    {
        myTouch.read();
        x = myTouch.getX();
        y = myTouch.getY();

        // Start Scan
        if ((x >= 145) && (x <= 308))
        {
            if ((y >= 80) && (y <= 130))
            {
                waitForIt(145, 80, 308, 130);
                // PIDSCAN
                page = 10;
                hasDrawn = false;
            }
            if ((y >= 135) && (y <= 185))
            {
                waitForIt(145, 135, 308, 185);
                // PID Guages
                page = 12;
                hasDrawn = false;
            }
            if ((y >= 190) && (y <= 240))
            {
                waitForIt(145, 190, 308, 240);
                // DTC Scan
                page = 14;
                hasDrawn = false;
            }
            if ((y >= 245) && (y <= 295))
            {
                //waitForIt(145, 245, 308, 295);
                // Unused
                //page = 16;
                //hasDrawn = false;
            }
        }
        if ((x >= 312) && (x <= 475))
        {
            if ((y >= 80) && (y <= 130))
            {
                waitForIt(312, 80, 475, 130);
                // PIDSTRM
                page = 11;
                hasDrawn = false;
            }
            if ((y >= 135) && (y <= 185))
            {
                waitForIt(312, 135, 475, 185);
                // VIN
                page = 13;
                hasDrawn = false;
            }
            if ((y >= 190) && (y <= 240))
            {
                waitForIt(312, 190, 475, 240);
                // DTC Clear
                page = 15;
                hasDrawn = false;
            }
            if ((y >= 245) && (y <= 295))
            {
                //waitForIt(312, 245, 475, 295);
                // Unused
                //page = 17;
                //hasDrawn = false;
            }
        }
    }
}

/*========== PID Scan Functions ==========*/
void drawPIDSCAN()
{
    drawSquareBtn(145, 60, 479, 319, "", themeBackground, themeBackground, themeBackground, CENTER);
    drawSquareBtn(141, 80, 479, 100, "Scan supported PIDs", themeBackground, themeBackground, menuBtnColor, CENTER);
    drawSquareBtn(141, 105, 479, 125, "to SD Card", themeBackground, themeBackground, menuBtnColor, CENTER);
    drawRoundBtn(200, 135, 400, 185, "Start Scan", menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
}

void PIDSCAN()
{
    while (true)
    {
        // Touch screen controls
        if (myTouch.dataAvailable())
        {
            myTouch.read();
            x = myTouch.getX();
            y = myTouch.getY();

            // Start Scan
            if ((x >= 200) && (x <= 400))
            {
                if ((y >= 135) && (y <= 185))
                {
                    waitForIt(200, 135, 400, 185);
                    startPIDSCAN();
                }
            }
        }
        return;
    }
}

void startPIDSCAN()
{
    bool hasNext = true;
    uint8_t status = 1;
    uint8_t bank = 0;
    uint16_t rxid = 0x7E8;
    uint8_t range = 0x00;

    // Print loading bar with current status
    loadBar(status);
    // Get vehicle vin, will be saved in can1 object
    can1.requestVIN(rxid, currentDir, true);

    // Update load bar
    loadBar(status++);

    // Get run time
    unsigned long timer = millis();

    // Loop though all available banks of PIDS
    while (hasNext && (millis() - timer < 30000))
        //while (hasNext && (millis() - timer < 2000))
    {
        // Get PID list with current range and bank
        can1.getPIDList(range, bank);

        // Update load bar
        loadBar(status++);

        delay(100);
        range = range + 0x20;
        bank++;

        // Check last bit to see if there are more PIDs in the next bank
        hasNext = can1.getNextPID();
    }

    if (millis() - timer < 30000)
    {
        // Complete load bar
        loadBar(DONE);

        // Activate PIDSTRM page 
        hasPID = true;
    }

    return;
}

/*========== PID Stream Functions ==========*/
void drawPIDStreamScroll(int scroll)
{
    // Temp to hold PIDS value before strcat
    char temp[2];

    // Starting y location for list
    int y = 60;

    // Draw the scroll window
    for (int i = 0; i < MAXSCROLL; i++)
    {
        char intOut[4] = "0x";
        itoa(arrayIn[scroll + 1], temp, 16);
        strcat(intOut, temp);
        if (scroll < sizeof(arrayIn) && arrayIn[scroll + 1] > 0)
        {
            drawSquareBtn(150, y, 410, y + 35, intOut, menuBackground, menuBtnBorder, menuBtnText, LEFT);
        }
        else
        {
            drawSquareBtn(150, y, 410, y + 35, "", menuBackground, menuBtnBorder, menuBtnText, LEFT);
        }
        y = y + 35;
        scroll++;
    }
}

void drawPIDStream(int scroll = 0)
{
    drawSquareBtn(141, 60, 479, 319, "", themeBackground, themeBackground, themeBackground, CENTER);
    int k = scroll;
    myGLCD.setColor(menuBtnColor);
    myGLCD.setBackColor(themeBackground);
    drawSquareBtn(420, 100, 470, 150, "/\\", menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
    drawSquareBtn(420, 150, 470, 200, "\\/", menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
    drawPIDStreamScroll(scroll);
    drawRoundBtn(150, 275, 400, 315, "Stream PID", menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
}

// Button function for the program page
void PIDStreamButtons()
{
    static uint8_t selected = 0;
    static int scroll = 0;
    // Touch screen controls
    if (myTouch.dataAvailable())
    {
        myTouch.read();
        x = myTouch.getX();
        y = myTouch.getY();

        if ((x >= 150) && (x <= 410))
        {
            if ((y >= 60) && (y <= 95))
            {
                waitForItRect(150, 60, 410, 95);
                Serial.println(1 + scroll);
                selected = 1 + scroll;
            }
            if ((y >= 95) && (y <= 130))
            {
                waitForItRect(150, 95, 410, 130);
                Serial.println(2 + scroll);
                selected = 2 + scroll;
            }
            if ((y >= 130) && (y <= 165))
            {
                waitForItRect(150, 130, 410, 165);
                Serial.println(3 + scroll);
                selected = 3 + scroll;
            }
            if ((y >= 165) && (y <= 200))
            {
                waitForItRect(150, 165, 410, 200);
                Serial.println(4 + scroll);
                selected = 4 + scroll;
            }
            if ((y >= 200) && (y <= 235))
            {
                waitForItRect(150, 200, 410, 235);
                Serial.println(5 + scroll);
                selected = 5 + scroll;
            }
            if ((y >= 235) && (y <= 270))
            {
                waitForItRect(150, 235, 410, 270);
                Serial.println(6 + scroll);
                selected = 6 + scroll;
            }
        }
        if ((x >= 420) && (x <= 470))
        {
            if ((y >= 100) && (y <= 150))
            {
                waitForItRect(420, 100, 470, 150);
                if (scroll > 0)
                {
                    scroll = scroll - 6;
                    drawPIDStreamScroll(scroll);
                }
            }
        }
        if ((x >= 420) && (x <= 470))
        {
            if ((y >= 150) && (y <= 200))
            {
                waitForItRect(420, 150, 470, 200);
                if (scroll < 100)
                {
                    scroll = scroll + 6;
                    drawPIDStreamScroll(scroll);
                }
            }
        }
        if ((x >= 150) && (x <= 400))
        {
            if ((y >= 275) && (y <= 315))
            {
                if (selected != 0)
                {
                    waitForItRect(150, 275, 400, 315);
                    Serial.print("Sending PID: ");
                    Serial.println(arrayIn[selected + 1]);
                    can1.PIDStream(CAN_PID_ID, arrayIn[selected]);
                }
            }
        }
    }
    return;
}

/*============== PID Stream Guages ==============*/
void PIDGauges()
{
    bool isWait = true;

    drawSquareBtn(145, 60, 479, 319, "", themeBackground, themeBackground, themeBackground, CENTER);
    //drawSquareBtn(145, 140, 479, 160, "Spare function", themeBackground, themeBackground, menuBtnColor, CENTER);
    myGLCD.setBackColor(menuBtnColor);
    myGLCD.setColor(menuBtnColor);
    myGLCD.fillCircle(380, 250, 60);
    myGLCD.fillCircle(380, 120, 60);
    myGLCD.fillCircle(220, 250, 60);
    myGLCD.fillCircle(220, 120, 60);
    myGLCD.setBackColor(VGA_WHITE);
    myGLCD.setColor(VGA_WHITE);
    myGLCD.fillCircle(380, 250, 55);
    myGLCD.fillCircle(380, 120, 55);
    myGLCD.fillCircle(220, 250, 55);
    myGLCD.fillCircle(220, 120, 55);

    myGLCD.setBackColor(VGA_WHITE);
    myGLCD.setColor(menuBtnColor);
    myGLCD.print("Load", 188, 148); // 4
    myGLCD.print("RPM", 358, 148);// C
    myGLCD.print("TEMP", 188, 278); // 5
    myGLCD.print("MPH", 358, 278); // D
    myGLCD.setBackColor(menuBtnColor);

    float pi = 3.14159;
    int r = 50;

    float x1 = r * cos(pi / 2) + 220;
    float y1 = r * sin(pi / 2) + 120;

    float x2 = r * cos(pi / 2) + 380;
    float y2 = r * sin(pi / 2) + 120;

    float x3 = r * cos(pi / 2) + 220;
    float y3 = r * sin(pi / 2) + 250;

    float x4 = r * cos(pi / 2) + 380;
    float y4 = r * sin(pi / 2) + 250;

    /*
    for (float t = pi / 2 + 1; t <= 2.5 * pi - 1; t+= 0.015)// 286 points
    {
        myGLCD.setBackColor(VGA_WHITE);
        myGLCD.setColor(VGA_WHITE);
        myGLCD.drawLine(380, 250, x, y);
        x = r * cos(t) + 380;
        y = r * sin(t) + 250;
        myGLCD.setBackColor(menuBtnColor);
        myGLCD.setColor(menuBtnColor);
        myGLCD.drawLine(380, 250, x, y);
        delay(10);
    }
    */
    float offset = (pi / 2) + 1;
    float g1, g2, g3, g4 = -1;
    while (isWait)
    {
        if (myTouch.dataAvailable())
        {
            myTouch.read();
            x = myTouch.getX();
            y = myTouch.getY();

            if ((x >= 1) && (x <= 140))
            {
                if ((y >= 1) && (y <= 319))
                {
                    isWait = false;
                }
            }
        }
        g1 = can1.PIDStreamGauge(CAN_PID_ID, 0x4);
        g2 = can1.PIDStreamGauge(CAN_PID_ID, 0xC);
        g3 = can1.PIDStreamGauge(CAN_PID_ID, 0x5);
        g4 = can1.PIDStreamGauge(CAN_PID_ID, 0xD);
        myGLCD.setBackColor(VGA_WHITE);
        myGLCD.printNumI(g1, 197, 128, 3, '0');
        myGLCD.printNumI(g2, 343, 128, 5, '0');
        myGLCD.printNumI(g3, 193, 258, 3, '0');
        myGLCD.printNumI(g4, 358, 258, 3, '0');

        // gauge values 0-286

        if (g1 >= 0)
        {
            g1 = offset + (g1 * 0.042);
            myGLCD.setBackColor(VGA_WHITE);
            myGLCD.setColor(VGA_WHITE);
            myGLCD.drawLine(220, 120, x1, y1);
            x1 = r * cos(g1) + 220;
            y1 = r * sin(g1) + 120;
            myGLCD.setBackColor(menuBtnColor);
            myGLCD.setColor(menuBtnColor);
            myGLCD.drawLine(220, 120, x1, y1);
        }

        if (g2 >= 0)
        {
            g2 = (offset + ((g2 * 0.0286) * 0.015));
            myGLCD.setBackColor(VGA_WHITE);
            myGLCD.setColor(VGA_WHITE);
            myGLCD.drawLine(380, 120, x2, y2);
            x2 = r * cos(g2) + 380;
            y2 = r * sin(g2) + 120;
            myGLCD.setBackColor(menuBtnColor);
            myGLCD.setColor(menuBtnColor);
            myGLCD.drawLine(380, 120, x2, y2);
        }

        if (g3 >= 0)
        {
            g3 = offset + (g3 * 0.01);
            myGLCD.setBackColor(VGA_WHITE);
            myGLCD.setColor(VGA_WHITE);
            myGLCD.drawLine(220, 250, x3, y3);
            x3 = r * cos(g3) + 220;
            y3 = r * sin(g3) + 250;
            myGLCD.setBackColor(menuBtnColor);
            myGLCD.setColor(menuBtnColor);
            myGLCD.drawLine(220, 250, x3, y3);
        }

        if (g4 >= 0)
        {
            g4 = offset + (g4 * 0.027);
            myGLCD.setBackColor(VGA_WHITE);
            myGLCD.setColor(VGA_WHITE);
            myGLCD.drawLine(380, 250, x4, y4);
            x4 = r * cos(g4) + 380;
            y4 = r * sin(g4) + 250;
            myGLCD.setBackColor(menuBtnColor);
            myGLCD.setColor(menuBtnColor);
            myGLCD.drawLine(380, 250, x4, y4);
        }
    }
    return;
}

/*================ Draw Vin ================*/
void drawVIN()
{
    uint16_t rxid = 0x7E8;
    drawSquareBtn(145, 60, 479, 319, "", themeBackground, themeBackground, themeBackground, CENTER);
    can1.requestVIN(rxid, currentDir, false);
    drawSquareBtn(150, 150, 479, 170, "VIN", themeBackground, themeBackground, menuBtnColor, CENTER);
    drawSquareBtn(150, 180, 479, 200, can1.getVIN(), themeBackground, themeBackground, menuBtnColor, CENTER);
}
/*============== DTC SCAN ==============*/
// Draw
// Function
void clearDTC()
{
    uint32_t IDc[7] = { 0x7D0, 0x720, 0x765, 0x737, 0x736, 0x721, 0x760 };
    byte MSGc[8] = { 0x4, 0x18, 0x00, 0xFF, 0x00, 0x55, 0x55, 0x55 };
    drawSquareBtn(145, 60, 479, 319, "", themeBackground, themeBackground, themeBackground, CENTER);
    drawSquareBtn(150, 150, 479, 170, "Clearing DTCS...", themeBackground, themeBackground, menuBtnColor, CENTER);
    for (int i = 0; i < 7; i++)
    {
        for (int j = 0; j < 3; j++)
        {
            can1.sendFrame(IDc[i], MSGc);
            delay(400);
        }
    }
    drawSquareBtn(150, 150, 479, 170, "All DTCS Cleared", themeBackground, themeBackground, menuBtnColor, CENTER);
}
// Buttons

/*============== DTC CLEAR ==============*/
// Draw
// Function
// Buttons


/*=========================================================
    Polaris Tools
===========================================================*/
void drawRZRTOOL()
{
    drawSquareBtn(145, 60, 479, 319, "", themeBackground, themeBackground, themeBackground, CENTER);
    drawRoundBtn(145, 80, 308, 130, "Scan DTC", menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
    drawRoundBtn(312, 80, 475, 130, "Clear DTC", menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
    //drawRoundBtn(145, 135, 308, 185, "Unused", menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
    //drawRoundBtn(312, 135, 475, 185, "Unused", menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
    //drawRoundBtn(145, 190, 308, 240, "Unused", menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
    //drawRoundBtn(312, 190, 475, 240, "Unused", menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
    //drawRoundBtn(145, 245, 308, 295, "Unused", menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
    //drawRoundBtn(312, 245, 475, 295, "Unused", menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
    drawSquareBtn(150, 300, 479, 319, VERSION, themeBackground, themeBackground, menuBtnColor, CENTER);
}

void RZRToolButtons()
{
    // Touch screen controls
    if (myTouch.dataAvailable())
    {
        myTouch.read();
        x = myTouch.getX();
        y = myTouch.getY();

        // Start Scan
        if ((x >= 145) && (x <= 308))
        {
            if ((y >= 80) && (y <= 130))
            {
                waitForIt(145, 80, 308, 130);
                // Scan DTC
                page = 19;
                hasDrawn = false;
            }
            if ((y >= 135) && (y <= 185))
            {
                waitForIt(145, 135, 308, 185);
                // Unused
                //page = 21;
                //hasDrawn = false;
            }
            if ((y >= 190) && (y <= 240))
            {
                waitForIt(145, 190, 308, 240);
                // Unused
                //page = 23;
                //hasDrawn = false;
            }
            if ((y >= 245) && (y <= 295))
            {
                waitForIt(145, 245, 308, 295);
                // Unused
                //page = 25;
                //hasDrawn = false;
            }
        }
        if ((x >= 312) && (x <= 475))
        {
            if ((y >= 80) && (y <= 130))
            {
                waitForIt(312, 80, 475, 130);
                // Clear DTC
                page = 20;
                hasDrawn = false;
            }
            if ((y >= 135) && (y <= 185))
            {
                waitForIt(312, 135, 475, 185);
                // Unused
                //page = 22;
                //hasDrawn = false;

            }
            if ((y >= 190) && (y <= 240))
            {
                waitForIt(312, 190, 475, 240);
                // Unused
                //page = 24;
                //hasDrawn = false;
            }
            if ((y >= 245) && (y <= 295))
            {
                waitForIt(312, 245, 475, 295);
                // Unused
                //page = 26;
                //hasDrawn = false;
            }
        }
    }
}

/*============== DTC SCAN ==============*/
// Draw
// Function
// Buttons

/*============== DTC CLEAR ==============*/
// Draw
// Function
// Buttons


/*=========================================================
    Extra Functions
===========================================================*/
void drawExtraFN()
{
    drawSquareBtn(145, 60, 479, 319, "", themeBackground, themeBackground, themeBackground, CENTER);
    drawRoundBtn(145, 80, 308, 130, "AFM", menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
    drawRoundBtn(312, 80, 475, 130, "StartStop", menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
    //drawRoundBtn(145, 135, 308, 185, "Unused", menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
    //drawRoundBtn(312, 135, 475, 185, "Unused", menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
    //drawRoundBtn(145, 190, 308, 240, "Unused", menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
    //drawRoundBtn(312, 190, 475, 240, "Unused", menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
    //drawRoundBtn(145, 245, 308, 295, "Unused", menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
    //drawRoundBtn(312, 245, 475, 295, "Unused", menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
    drawSquareBtn(150, 300, 479, 319, VERSION, themeBackground, themeBackground, menuBtnColor, CENTER);
}

void extraFNButtons()
{
    // Touch screen controls
    if (myTouch.dataAvailable())
    {
        myTouch.read();
        x = myTouch.getX();
        y = myTouch.getY();

        // Start Scan
        if ((x >= 145) && (x <= 308))
        {
            if ((y >= 80) && (y <= 130))
            {
                waitForIt(145, 80, 308, 130);
                // AFM
                page = 28;
                hasDrawn = false;
            }
            if ((y >= 135) && (y <= 185))
            {
                //waitForIt(145, 135, 308, 185);
                // Unused
                // page = 30;
                // hasDrawn = false;
            }
            if ((y >= 190) && (y <= 240))
            {
                //waitForIt(145, 190, 308, 240);
                // Unused
                //page = 32;
                //hasDrawn = false;
            }
            if ((y >= 245) && (y <= 295))
            {
                //waitForIt(145, 245, 308, 295);
                // Unused
                //page = 34;
                //hasDrawn = false;
            }
        }
        if ((x >= 312) && (x <= 475))
        {
            if ((y >= 80) && (y <= 130))
            {
                waitForIt(312, 80, 475, 130);
                // autoStartStop
                page = 29;
                hasDrawn = false;
            }
            if ((y >= 135) && (y <= 185))
            {
                //waitForIt(312, 135, 475, 185);
                // Unused
                //page = 31;
                //hasDrawn = false;

            }
            if ((y >= 190) && (y <= 240))
            {
                //waitForIt(312, 190, 475, 240);
                // Unused
                //page = 33;
                //hasDrawn = false;
            }
            if ((y >= 245) && (y <= 295))
            {
                //waitForIt(312, 245, 475, 295);
                // Unused
                //page = 35;
                //hasDrawn = false;
            }
        }
    }
}

// Custom vehicle testing functions / scripts
void autoStartStop()
{
    byte test1[8] = { 0x07, 0xAE, 0x10, 0x00, 0x00, 0x00, 0x10, 0x00 };
    byte test2[8] = { 0xFE, 0x01, 0x3E, 0x00, 0x00, 0x00, 0x00, 0x00 };

    can1.sendFrame(0x7E0, test1);
    delay(3);

    uint32_t timer1 = millis();
    uint32_t timer2 = millis();

    while (page == 29)
    {
        menuButtons();

        if (millis() - timer1 >= 2000)
        {
            can1.sendFrame(0x7E0, test1);
            timer1 = millis();
        }
        if (millis() - timer2 >= 500)
        {
            can1.sendFrame(0x101, test2);
            timer2 = millis();
        }

    }

}


void AFM()
{
    byte test1[8] = { 0x07, 0xAE, 0x10, 0x00, 0x00, 0x00, 0x10, 0x00 };
    byte test2[8] = { 0xFE, 0x01, 0x3E, 0x00, 0x00, 0x00, 0x00, 0x00 };

    can1.sendFrame(0x7E0, test1);
    delay(2);

    uint32_t timer1 = millis();

    while (page == 28)
    {
        menuButtons();

        if (millis() - timer1 >= 2000)
        {
            can1.sendFrame(0x101, test2);
            timer1 = millis();
        }
    }


}

/*
void clearDTC()
{
    byte test1[8] = { 0x02, 0x10, 0x92, 0x55, 0x55, 0x55, 0x55, 0x55 };
    byte test2[8] = { 0x02, 0x10, 0x03, 0x55, 0x55, 0x55, 0x55, 0x55 };
    can1.sendFrame(0x7E1, test1);
    can1.sendFrame(0x7E1, test2);
}
*/

/*=========================================================
    Settings
===========================================================*/
void drawSettings()
{
    drawSquareBtn(145, 60, 479, 319, "", themeBackground, themeBackground, themeBackground, CENTER);
    drawRoundBtn(145, 80, 308, 130, "Fltr CAN0", menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
    drawRoundBtn(312, 80, 475, 130, "Fltr CAN1", menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
    drawRoundBtn(145, 135, 308, 185, "Set Baud0", menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
    drawRoundBtn(312, 135, 475, 185, "Set Baud1", menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
    //drawRoundBtn(145, 190, 308, 240, "Unused", menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
    //drawRoundBtn(312, 190, 475, 240, "Unused", menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
    //drawRoundBtn(145, 245, 308, 295, "Unused", menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
    //drawRoundBtn(312, 245, 475, 295, "Unused", menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
    drawSquareBtn(150, 300, 479, 319, VERSION, themeBackground, themeBackground, menuBtnColor, CENTER);
}

void settingsButtons()
{
    // Touch screen controls
    if (myTouch.dataAvailable())
    {
        myTouch.read();
        x = myTouch.getX();
        y = myTouch.getY();

        // Start Scan
        if ((x >= 145) && (x <= 308))
        {
            if ((y >= 80) && (y <= 130))
            {
                waitForIt(145, 80, 308, 130);
                // Filter CAN0
                page = 37;
                hasDrawn = false;
            }
            if ((y >= 135) && (y <= 185))
            {
                waitForIt(145, 135, 308, 185);
                // Set Baud0
                page = 39;
                hasDrawn = false;
            }
            if ((y >= 190) && (y <= 240))
            {
                //waitForIt(145, 190, 308, 240);
                // Unused
                //page = 41;
                //hasDrawn = false;
            }
            if ((y >= 245) && (y <= 295))
            {
                //waitForIt(145, 245, 308, 295);
                // Unused
                //page = 43;
                //hasDrawn = false;
            }
        }
        if ((x >= 312) && (x <= 475))
        {
            if ((y >= 80) && (y <= 130))
            {
                waitForIt(312, 80, 475, 130);
                // Filter CAN1
                page = 38;
                hasDrawn = false;
            }
            if ((y >= 135) && (y <= 185))
            {
                waitForIt(312, 135, 475, 185);
                // Set Baud1
                page = 40;
                hasDrawn = false;
            }
            if ((y >= 190) && (y <= 240))
            {
                //waitForIt(312, 190, 475, 240);
                // Unused
                //page = 42;
                //hasDrawn = false;
            }
            if ((y >= 245) && (y <= 295))
            {
                //waitForIt(312, 245, 475, 295);
                // Unused
                //page = 44;
                //hasDrawn = false;

            }
        }
    }
}

// Draw hex number pad
void drawNumpad()
{
    // Clear background
    drawSquareBtn(145, 60, 479, 319, "", themeBackground, themeBackground, themeBackground, CENTER);

    // Print page title
    drawSquareBtn(180, 57, 460, 77, "CAN Filter Range", themeBackground, themeBackground, menuBtnColor, CENTER);

    //
    int posY = 80;
    uint8_t numPad = 0x00;

    for (uint8_t i = 0; i < 3; i++)
    {
        int posX = 145;
        for (uint8_t j = 0; j < 6; j++)
        {
            if (numPad < 0x10)
            {
                drawRoundBtn(posX, posY, posX + 50, posY + 40, String(numPad, HEX), menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
                posX += 55;
                numPad++;
            }
        }
        posY += 45;
    }
    drawRoundBtn(365, 170, 470, 210, "Clear", menuBtnColor, menuBtnBorder, menuBtnText, CENTER);

    drawRoundBtn(145, 220, 305, 260, String(startRange, 16), menuBtnColor, menuBtnBorder, menuBtnText, LEFT);
    drawRoundBtn(310, 220, 470, 260, String(endRange, 16), menuBtnColor, menuBtnBorder, menuBtnText, LEFT);

    drawRoundBtn(145, 270, 305, 310, "Accept", menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
    drawRoundBtn(315, 270, 470, 310, "Cancel", menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
}

// Buttons for number pad
int numpadButtons()
{
    // Touch screen controls
    if (myTouch.dataAvailable())
    {
        myTouch.read();
        x = myTouch.getX();
        y = myTouch.getY();

        if ((y >= 80) && (y <= 120))
        {
            // 0
            if ((x >= 145) && (x <= 195))
            {
                waitForIt(145, 80, 195, 120);
                return 0x00;
            }
            // 1
            if ((x >= 200) && (x <= 250))
            {
                waitForIt(200, 80, 250, 120);
                return 0x01;
            }
            // 2
            if ((x >= 255) && (x <= 305))
            {
                waitForIt(255, 80, 305, 120);
                return 0x02;
            }
            // 3
            if ((x >= 310) && (x <= 360))
            {
                waitForIt(310, 80, 360, 120);
                return 0x03;
            }
            // 4
            if ((x >= 365) && (x <= 415))
            {
                waitForIt(365, 80, 415, 120);
                return 0x05;
            }
            // 5
            if ((x >= 420) && (x <= 470))
            {
                waitForIt(420, 80, 470, 120);
                return 0x05;
            }
        }
        if ((y >= 125) && (y <= 165))
        {
            // 6
            if ((x >= 145) && (x <= 195))
            {
                waitForIt(145, 125, 195, 165);
                return 0x06;
            }
            // 7
            if ((x >= 200) && (x <= 250))
            {
                waitForIt(200, 125, 250, 165);
                return 0x07;
            }
            // 8
            if ((x >= 255) && (x <= 305))
            {
                waitForIt(255, 125, 305, 165);
                return 0x08;
            }
            // 9
            if ((x >= 310) && (x <= 360))
            {
                waitForIt(310, 125, 360, 165);
                return 0x09;
            }
            // A
            if ((x >= 365) && (x <= 415))
            {
                waitForIt(365, 125, 415, 165);
                return 0x0A;
            }
            // B
            if ((x >= 420) && (x <= 470))
            {
                waitForIt(420, 125, 470, 165);
                return 0x0B;
            }
        }
        if ((y >= 170) && (y <= 210))
        {
            // C
            if ((x >= 145) && (x <= 195))
            {
                waitForIt(145, 170, 195, 210);
                return 0x0C;
            }
            // D
            if ((x >= 200) && (x <= 250))
            {
                waitForIt(200, 170, 250, 210);
                return 0x0D;
            }
            // E
            if ((x >= 255) && (x <= 305))
            {
                waitForIt(255, 170, 305, 210);
                return 0x0E;
            }
            // F
            if ((x >= 310) && (x <= 360))
            {
                waitForIt(310, 170, 360, 210);
                return 0x0F;

            }
            // Clear
            if ((x >= 365) && (x <= 470))
            {
                waitForIt(365, 170, 470, 210);
                return 0x10;
            }
        }
        if ((y >= 220) && (y <= 260))
        {
            // Start
            if ((x >= 145) && (x <= 305))
            {
                waitForIt(145, 220, 305, 260);
                return 0x12;
            }
            // End
            if ((x >= 310) && (x <= 470))
            {
                waitForIt(310, 220, 470, 260);
                return 0x13;
            }
        }
        if ((y >= 270) && (y <= 310))
        {
            // Accept
            if ((x >= 145) && (x <= 305))
            {
                waitForIt(145, 270, 305, 310);
                return 0x11;

            }
            // Cancel
            if ((x >= 315) && (x <= 470))
            {
                waitForIt(315, 270, 470, 310);
                return -2;
            }
        }
    }
    return -1;
}

//
void canFilter(bool CAN)
{
    uint8_t startPos = 0;
    uint8_t endPos = 0;
    uint8_t startArray[8] = { 0, 0, 0, 0, 0, 0, 0, 0 };
    uint8_t endArray[8] = { 0, 0, 0, 0, 0, 0, 0, 0 };
    int rValue = -1;

    uint8_t sLength = 0;
    uint8_t eLength = 0;
    bool selectedRange = false;
    while (page == 37 || page == 38)
    {
        uint32_t hexTable[8] = { 1, 16, 256, 4096, 65536, 1048576, 16777216, 268435456 };
        menuButtons();
        rValue = numpadButtons();
        delay(100);
        if (rValue == 0x12)
        {
            selectedRange = false;
        }
        if (rValue == 0x13)
        {
            selectedRange = true;
        }
        if (rValue >= 0x00 && rValue < 0x10)
        {
            Serial.print("rValue: ");
            Serial.println(rValue);
            switch (selectedRange)
            {
            case false:
                if (sLength == 0)
                {
                    // New value always goes to pos 0
                    startArray[0] = rValue;
                    sLength++;
                }
                else if (sLength < 8)
                {
                    // Shift array left as each new value comes in
                    for (uint8_t i = sLength; i > 0; i--)
                    {
                        startArray[i] = startArray[i - 1];
                    }
                    startArray[0] = rValue;
                    sLength++;
                }
                else
                {
                    // Full
                }
                startRange = 0;
                for (uint8_t i = 0; i < sLength; i++)
                {
                    Serial.print("startArray[i]: ");
                    Serial.println(startArray[i]);
                    startRange = startRange + (hexTable[i] * startArray[i]);
                    Serial.print("hexTable: ");
                    Serial.println(hexTable[i]);
                    Serial.print("startRange: ");
                    Serial.println(startRange);
                    Serial.println("");
                }

                drawRoundBtn(145, 220, 305, 260, String(startRange, 16), menuBtnColor, menuBtnBorder, menuBtnText, LEFT);
                break;
            case true:
                if (eLength == 0)
                {
                    // New value always goes to pos 0
                    endArray[0] = rValue;
                    eLength++;
                }
                else if (eLength < 8)
                {
                    // Shift array left as each new value comes in
                    for (uint8_t i = eLength; i > 0; i--)
                    {
                        endArray[i] = endArray[i - 1];
                    }
                    endArray[0] = rValue;
                    eLength++;
                }
                else
                {
                    // Full
                }
                endRange = 0;
                for (uint8_t i = 0; i < eLength; i++)
                {
                    endRange = endRange + (hexTable[i] * endArray[i]);
                }

                drawRoundBtn(310, 220, 470, 260, String(endRange, 16), menuBtnColor, menuBtnBorder, menuBtnText, LEFT);
                break;
            }
        }
        if (rValue == 0x10)
        {
            switch (selectedRange)
            {
            case false:
                for (uint8_t i = 0; i < 8; i++)
                {
                    startArray[i] = 0;
                }
                startRange = 0;
                sLength = 0;
                drawRoundBtn(145, 220, 305, 260, String(startRange, 16), menuBtnColor, menuBtnBorder, menuBtnText, LEFT);
                break;
            case true:
                for (uint8_t i = 0; i < 8; i++)
                {
                    endArray[i] = 0;
                }
                endRange = 0x1FFFFFFF;
                eLength = 0;
                drawRoundBtn(310, 220, 470, 260, String(endRange, 16), menuBtnColor, menuBtnBorder, menuBtnText, LEFT);
            }
        }
        if (rValue == 0x11)
        {
            if (CAN == true)
            {
                can1.setFilterMask0(startRange, endRange);
                Serial.println(startRange, HEX);
                Serial.println(endRange, HEX);
                page = 36;
                hasDrawn = false;
                return;
            }
            if (CAN == false)
            {
                can1.setFilterMask1(startRange, endRange);
                page = 37;
                hasDrawn = false;
                return;
            }
        }
        if (rValue == -2)
        {
            page = 36;
            hasDrawn = false;
            return;
        }
    }
}

// Set Baud
// Filter CAN0
// Filter CAN1


/*=========================================================
    Framework Functions
===========================================================*/
//Only called once at startup to draw the menu
void drawMenu()
{
    // Draw Layout
    drawSquareBtn(1, 1, 479, 319, "", themeBackground, themeBackground, themeBackground, CENTER);
    drawSquareBtn(1, 1, 140, 319, "", menuBackground, menuBackground, menuBackground, CENTER);

    // Draw Menu Buttons
      /*
    drawRoundBtn(10, 10, 130, 65, "HOME", menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
    drawRoundBtn(10, 70, 130, 125, "PIDSCAN", menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
    drawRoundBtn(10, 130, 130, 185, "PIDSTRM", menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
    drawRoundBtn(10, 190, 130, 245, "EXTRAFN", menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
    drawRoundBtn(10, 250, 130, 305, "TRAFFIC", menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
  */
    drawRoundBtn(10, 10, 130, 65, "CANBUS", menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
    drawRoundBtn(10, 70, 130, 125, "VEHTOOL", menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
    drawRoundBtn(10, 130, 130, 185, "RZRTOOL", menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
    drawRoundBtn(10, 190, 130, 245, "EXTRAFN", menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
    drawRoundBtn(10, 250, 130, 305, "SETTING", menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
}

//Manages the different App pages
int pageControl()
{
    while (true)
    {
        // Check menu buttons for input
        menuButtons();
        switch (page)
        {
        case 0: /*========== CANBUS ==========*/
            if (!hasDrawn)
            {
                hasDrawn = true;
                // Draw Page
                drawCANBus();
            }
            // Call buttons if any
            CANBusButtons();
            break;
        case 1:
            if (!hasDrawn)
            {
                hasDrawn = true;
                // Draw Page
                drawReadInCANLCD();
            }
            // Call buttons if any
            readInCANMsg(CAN0);
            break;
        case 2:
            if (!hasDrawn)
            {
                hasDrawn = true;
                // Draw Page
                drawCANSerial();
            }
            // Call buttons if any
            can1.CANTraffic();
            break;
        case 3:
            if (!hasDrawn)
            {
                hasDrawn = true;
                // Draw Page
                drawReadInCANLCD();
            }
            // Call buttons if any
            readInCANMsg(CAN1);
            break;
        case 4:
            if (!hasDrawn)
            {
                hasDrawn = true;
                // Draw Page
                drawCANSerial();
            }
            // Call buttons if any
            can1.readCAN0TX();
            break;
        case 5:
            if (!hasDrawn)
            {
                hasDrawn = true;
                // Draw Page
            }
            // Call buttons if any
            break;
        case 6:
            if (!hasDrawn)
            {
                hasDrawn = true;
                // Draw Page
            }
            // Call buttons if any
            break;
        case 7:
            if (!hasDrawn)
            {
                hasDrawn = true;
                // Draw Page
            }
            // Call buttons if any
            break;
        case 8:
            if (!hasDrawn)
            {
                hasDrawn = true;
                // Draw Page
            }
            // Call buttons if any
            break;

        case 9: /*========== VEHTOOL ==========*/
            if (!hasDrawn)
            {
                hasDrawn = true;
                // Draw Page
                drawVehicleTools();
            }
            // Call buttons if any
            VehicleToolButtons();
            break;
        case 10: // PIDSCAN
            if (!hasDrawn)
            {
                hasDrawn = true;
                // Draw Page
                can1.startCAN0(0x7E0, 0x7EF);
                drawPIDSCAN();
            }
            // Call buttons if any
            PIDSCAN();
            break;
        case 11: // PIDSTRM
            if (!hasDrawn)
            {
                if (hasPID == true)
                {
                    can1.startCAN0(0x7E0, 0x7EF);
                    for (int i = 0; i < 100; i++)
                    {
                        arrayIn[i] = 0x00;
                    }
                    sdCard.readFile(can1.getFullDir(), arrayIn);
                    drawPIDStream();
                    hasDrawn = true;
                }
                else
                {
                    controlPage = 9;
                    errorMSG("Error", "Please Perform", "PIDSCAN First");
                    hasDrawn = true;
                }
            }
            // Call buttons if any
            if (hasPID == true)
            {
                PIDStreamButtons();
            }
            else
            {
                errorMSGButton(9);
            }
            break;

        case 12:
            if (!hasDrawn)
            {
                hasDrawn = true;
                // Draw Page
            }
            // Call buttons if any
            PIDGauges();
            break;
        case 13:
            if (!hasDrawn)
            {
                hasDrawn = true;
                // Draw Page
                drawVIN();
            }
            // Call buttons if any
            break;
        case 14: // 
            if (!hasDrawn)
            {
                hasDrawn = true;
                // Draw Page
            }
            // Call buttons if any
            break;
        case 15:
            if (!hasDrawn)
            {
                hasDrawn = true;
                // Draw Page
                clearDTC();
                page = 9;
            }
            // Call buttons if any
            break;
        case 16:
            if (!hasDrawn)
            {
                hasDrawn = true;
                // Draw Page
            }
            // Call buttons if any
            break;
        case 17:
            if (!hasDrawn)
            {
                hasDrawn = true;
                // Draw Page
            }
            // Call buttons if any
            break;

        case 18: /*========== RZRTOOL ==========*/
            if (!hasDrawn)
            {
                hasDrawn = true;
                // Draw Page
                drawRZRTOOL();
            }
            // Call buttons if any
            RZRToolButtons();
            break;
        case 19:
            if (!hasDrawn)
            {
                hasDrawn = true;
                // Draw Page
            }
            // Call buttons if any
            break;
        case 20:
            if (!hasDrawn)
            {
                hasDrawn = true;
                // Draw Page
            }
            // Call buttons if any
            break;
        case 21:
            if (!hasDrawn)
            {
                hasDrawn = true;
                // Draw Page
            }
            // Call buttons if any
            break;
        case 22:
            if (!hasDrawn)
            {
                hasDrawn = true;
                // Draw Page
            }
            // Call buttons if any
            break;
        case 23:
            if (!hasDrawn)
            {
                hasDrawn = true;
                // Draw Page
            }
            // Call buttons if any
            break;
        case 24:
            if (!hasDrawn)
            {
                hasDrawn = true;
                // Draw Page
            }
            // Call buttons if any
            break;
        case 25:
            if (!hasDrawn)
            {
                hasDrawn = true;
                // Draw Page
            }
            // Call buttons if any
            break;
        case 26:
            if (!hasDrawn)
            {
                hasDrawn = true;
                // Draw Page
            }
            // Call buttons if any
            break;

        case 27: /*========== EXTRAFN ==========*/
            if (!hasDrawn)
            {
                hasDrawn = true;
                // Draw Page
                drawExtraFN();
            }
            // Call buttons if any
            extraFNButtons();
            break;
        case 28:
            if (!hasDrawn)
            {
                hasDrawn = true;
                // Draw Page
                AFM();
            }
            // Call buttons if any
            break;
        case 29:
            if (!hasDrawn)
            {
                hasDrawn = true;
                // Draw Page
                autoStartStop();
            }
            // Call buttons if any
            break;
        case 30:
            if (!hasDrawn)
            {
                hasDrawn = true;
                // Draw Page
            }
            // Call buttons if any
            break;
        case 31:
            if (!hasDrawn)
            {
                hasDrawn = true;
                // Draw Page
            }
            // Call buttons if any
            break;
        case 32:
            if (!hasDrawn)
            {
                hasDrawn = true;
                // Draw Page
            }
            // Call buttons if any
            break;
        case 33:
            if (!hasDrawn)
            {
                hasDrawn = true;
                // Draw Page
            }
            // Call buttons if any
            break;
        case 34:
            if (!hasDrawn)
            {
                hasDrawn = true;
                // Draw Page
            }
            // Call buttons if any
            break;
        case 35:
            if (!hasDrawn)
            {
                hasDrawn = true;
                // Draw Page
            }
            // Call buttons if any
            break;

        case 36: /*========== SETTINGS ==========*/
            if (!hasDrawn)
            {
                hasDrawn = true;
                // Draw Page
                drawSettings();
            }
            // Call buttons if any
            settingsButtons();
            break;
        case 37:
            if (!hasDrawn)
            {
                hasDrawn = true;
                // Draw Page
                drawNumpad();
            }
            canFilter(true);
            break;
        case 38:
            if (!hasDrawn)
            {
                hasDrawn = true;
                // Draw Page
                drawNumpad();
            }
            // Call buttons if any
            canFilter(false);
            break;
        case 39:
            if (!hasDrawn)
            {
                hasDrawn = true;
                // Draw Page
                uint32_t temp = can1.getBaud();
                if (temp == 500000)
                {
                    can1.setBaud(250000);
                    //can1.startCAN0(startRange, endRange);
                    errorMSG("Baud Rate", "Set to:", String(can1.getBaud()));
                }
                else if (temp == 250000)
                {
                    can1.setBaud(500000);
                    //can1.startCAN0(startRange, endRange);
                    errorMSG("Baud Rate", "Set to:", String(can1.getBaud()));
                }
            }
            // Call buttons if any
            errorMSGButton(36);
            break;
        case 40:
            if (!hasDrawn)
            {
                hasDrawn = true;
                // Draw Page
                uint32_t temp = can1.getBaud();
                if (temp == 500000)
                {
                    can1.setBaud(250000);
                    //can1.startCAN0(startRange, endRange);
                    errorMSG("Baud Rate", "Set to:", String(can1.getBaud()));
                }
                else if (temp == 250000)
                {
                    can1.setBaud(500000);
                    //can1.startCAN0(startRange, endRange);
                    errorMSG("Baud Rate", "Set to:", String(can1.getBaud()));
                }
            }
            // Call buttons if any
            errorMSGButton(36);
            break;
        case 41:
            if (!hasDrawn)
            {
                hasDrawn = true;
                // Draw Page
            }
            // Call buttons if any
            break;
        case 42:
            if (!hasDrawn)
            {
                hasDrawn = true;
                // Draw Page
            }
            // Call buttons if any
            break;
        case 43:
            if (!hasDrawn)
            {
                hasDrawn = true;
                // Draw Page
            }
            // Call buttons if any

            break;
        case 44:
            if (!hasDrawn)
            {
                hasDrawn = true;
                // Draw Page
            }
            // Call buttons if any
            break;
        }
    }
}

// Error Message function
void errorMSG(String title, String eMessage1, String eMessage2)
{
    drawSquareBtn(170, 140, 450, 240, "", menuBackground, menuBtnColor, menuBtnColor, CENTER);
    drawSquareBtn(170, 140, 450, 170, title, themeBackground, menuBtnColor, menuBtnBorder, LEFT);
    drawSquareBtn(171, 171, 449, 204, eMessage1, menuBackground, menuBackground, menuBtnText, CENTER);
    drawSquareBtn(171, 205, 449, 239, eMessage2, menuBackground, menuBackground, menuBtnText, CENTER);
    drawRoundBtn(400, 140, 450, 170, "X", menuBtnColor, menuBtnColor, menuBtnText, CENTER);
}

// Error Message buttons
uint8_t errorMSGButton(uint8_t returnPage)
{
    // Touch screen controls
    if (myTouch.dataAvailable())
    {
        myTouch.read();
        x = myTouch.getX();
        y = myTouch.getY();

        if ((x >= 400) && (x <= 450))
        {
            if ((y >= 140) && (y <= 170))
            {
                waitForItRect(398, 138, 452, 172);
                page = returnPage;
                hasDrawn = false;
            }
        }
    }
}

// Button functions for main menu
void menuButtons()
{
    // Touch screen controls
    if (myTouch.dataAvailable())
    {
        myTouch.read();
        x = myTouch.getX();
        y = myTouch.getY();

        // Menu
        if ((x >= 10) && (x <= 130))  // Button: 1
        {
            if ((y >= 10) && (y <= 65))  // Upper row
            {
                waitForIt(10, 10, 130, 65);
                page = 0;
                hasDrawn = false;
            }
            if ((y >= 70) && (y <= 125))  // Upper row
            {

                // X_Start, Y_Start, X_Stop, Y_Stop
                waitForIt(10, 70, 130, 125);
                page = 9;
                hasDrawn = false;

            }
            if ((y >= 130) && (y <= 185))  // Upper row
            {
                // X_Start, Y_Start, X_Stop, Y_Stop
                waitForIt(10, 130, 130, 185);
                page = 18;
                hasDrawn = false;
            }
            // Settings touch button
            if ((y >= 190) && (y <= 245))
            {

                // X_Start, Y_Start, X_Stop, Y_Stop
                waitForIt(10, 190, 130, 245);
                page = 27;
                hasDrawn = false;
            }
            if ((y >= 250) && (y <= 305))
            {

                // X_Start, Y_Start, X_Stop, Y_Stop
                waitForIt(10, 250, 130, 305);
                page = 36;
                hasDrawn = false;
            }

        }
    }
    return;
}

// the setup function runs once when you press reset or power the board
void setup() {
    Serial.begin(115200);

    can1.startCAN0(0x000, 0x800);
    can1.startCAN1(0x000, 0x800);
    bool hasFailed = sdCard.startSD();
    if (!hasFailed)
    {
        Serial.println("SD failed");
    }
    else if (hasFailed)
    {
        Serial.println("SD Running");
    }

    // LCD  and touch screen settings
    myGLCD.InitLCD();
    myGLCD.clrScr();

    myTouch.InitTouch();
    myTouch.setPrecision(PREC_MEDIUM);

    myGLCD.setFont(BigFont);
    myGLCD.setBackColor(0, 0, 255);

    dispx = myGLCD.getDisplayXSize();
    dispy = myGLCD.getDisplayYSize();

    // Draw the main menu
    drawMenu();

    // Draw the Hypertech logo
    bmpDraw("System/HYPER.bmp", 0, 0);
}

// Calls pageControl with a value of 1 to set view page as the home page
void loop()
{
    controlPage = pageControl();
}

