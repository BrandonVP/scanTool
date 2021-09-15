/*
 Name:    scanTool.ino
 Created: 11/15/2020 8:27:18 AM
 Author:  Brandon Van Pelt
*/

/*=========================================================
    Todo List
===========================================================
Read Vehicle DTCs
Read / Clear RZR DTCs
Replace uint8_t scroll with var
===========================================================
    End Todo List
=========================================================*/

#include <DS3231.h>
#include <RTCDue.h>
#include <UTFT.h>
#include <SD.h>
#include <UTouchCD.h>
#include <memorysaver.h>
#include <SPI.h>
#include <UTouch.h>
#include "CANBus.h"
#include "definitions.h"
#include "SDCard.h"
#include <string.h>
#include "variables.h"

/*
Uncomment to update the clock then comment out and upload to 
the device a second time to prevent updating time to last time 
device was on every at every startup.
*/
//#define UPDATE_CLOCK

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

    int dispx = myGLCD.getDisplayXSize();
    int dispy = myGLCD.getDisplayYSize();

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
*           Draw Round/Square Button                 *
*                                                    *
*  Description:   Draws shapes with/without text     *
*  Parameters: int: x start, y start, x stop, y stop *
*              String: Button text                   *
*              Hex value: Background Color           *
*              Hex value: Border of shape            *
*              Hex value: Color of text              *
*              int: Alignment of text #defined as    *
*                   LEFT, CENTER, RIGHT              *
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
        drawSquareBtn(200, 200, 400, 220, F("Finished"), menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
        //myGLCD.setColor(menuBtnColor);
        //myGLCD.fillRect(200, 200, 400, 220);

        return;
    }
    drawSquareBtn(199, 199, 401, 221, "", themeBackground, menuBtnBorder, menuBtnText, LEFT);
    myGLCD.setColor(menuBtnColor);
    myGLCD.fillRect(200, 200, (200 + (progress * 25)), 220);
}

// Function to highlight buttons when clicked
void waitForItMenu(int x1, int y1, int x2, int y2)
{
    waitForItTimer = millis();

    if (!isWaitForIt)
    {
        myGLCD.setColor(themeBackground);
        myGLCD.drawRoundRect(x1, y1, x2, y2);
        x1_ = x1;
        y1_ = y1;
        x2_ = x2;
        y2_ = y2;
        isWaitForIt = true;
    }
}
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
    switch (graphicLoaderState)
    {
    case 0:
        break;
    case 1:
        drawSquareBtn(131, 55, 479, 319, "", themeBackground, themeBackground, themeBackground, CENTER);
        break;
    case 2:
        drawRoundBtn(145, 55, 308, 100, F("CAN0: LCD"), menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
        break;
    case 3:
        drawRoundBtn(312, 55, 475, 100, F("Serial"), menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
        break;
    case 4:
        drawRoundBtn(145, 105, 308, 150, F("CAN1: LCD"), menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
        break;
    case 5:
        drawRoundBtn(312, 105, 475, 150, F("Serial"), menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
        break;
    case 6:
        drawRoundBtn(145, 155, 308, 200, F("Both: LCD"), menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
        break;
    case 7:
        drawRoundBtn(312, 155, 475, 200, F("Serial"), menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
        break;
    case 8:
        drawRoundBtn(145, 205, 308, 250, F("CTX0: LCD"), menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
        break;
    case 9:
        drawRoundBtn(312, 205, 475, 250, F("Serial"), menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
        break;
    case 10:
        drawRoundBtn(145, 255, 308, 300, F("CAN0 RX"), menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
        break;
    case 11:
        drawRoundBtn(312, 255, 475, 300, F("CAN1 RX"), menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
        break;
    case 12:
        drawSquareBtn(150, 301, 479, 319, VERSION, themeBackground, themeBackground, menuBtnColor, CENTER);
        break;
    }
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
            if ((y >= 55) && (y <= 100))
            {
                waitForIt(145, 55, 308, 100);
                // CAN0: LCD
                page = 1;
                var1 = 60;
                selectedChannelOut = 1;
                hasDrawn = false;
            }
            if ((y >= 105) && (y <= 150))
            {
                waitForIt(145, 105, 308, 150);
                // CAN1: LCD
                page = 1;
                var1 = 60;
                selectedChannelOut = 2;
                hasDrawn = false;
            }
            if ((y >= 155) && (y <= 200))
            {
                waitForIt(145, 155, 308, 200);
                // Both: LCD
                page = 1;
                var1 = 60;
                selectedChannelOut = 3;
                hasDrawn = false;
            }
            if ((y >= 205) && (y <= 250))
            {
                waitForIt(145, 205, 308, 250);
                // CTX0: LCD
                page = 1;
                var1 = 60;
                selectedChannelOut = 4;
                hasDrawn = false;
            }
            if ((y >= 255) && (y <= 300))
            {
                waitForIt(145, 255, 308, 300);
                // CAN0 RX
                page = 3;
                hasDrawn = false;
            }
        }
        if ((x >= 312) && (x <= 475))
        {
            if ((y >= 55) && (y <= 100))
            {
                waitForIt(312, 55, 475, 100);
                // CAN0 Serial
                page = 2;
                hasDrawn = false;
                selectedChannelOut = 1;
            }
            if ((y >= 105) && (y <= 150))
            {
                waitForIt(312, 105, 475, 150);
                // CAN1 Serial
                page = 2;
                hasDrawn = false;
                selectedChannelOut = 2;
            }
            if ((y >= 155) && (y <= 200))
            {
                waitForIt(312, 155, 475, 200);
                // Both Serial
                page = 2;
                hasDrawn = false;
                selectedChannelOut = 3;
            }
            if ((y >= 205) && (y <= 250))
            {
                waitForIt(312, 205, 475, 250);
                // CTX0 Serial
                page = 2;
                hasDrawn = false;
                selectedChannelOut = 4;
            }
            if ((y >= 255) && (y <= 300))
            {
                waitForIt(312, 255, 475, 300);
                // CTX1 RX
                page = 4;
                hasDrawn = false;
            }
        }
    }
}

/*============== CAN: LCD ==============*/
void drawReadInCANLCD()
{
    drawSquareBtn(145, 55, 479, 319, "", themeBackground, themeBackground, themeBackground, CENTER);
}

void readInCANMsg(uint8_t channel)
{
    myGLCD.setBackColor(VGA_WHITE);
    myGLCD.setFont(SmallFont);
    uint8_t rxBuf[8];
    uint32_t rxId;
    uint8_t len;
    if (can1.LCDOutCAN(rxBuf, len, rxId, channel))
    {
        char printString[50];
        myGLCD.setColor(VGA_WHITE);
        myGLCD.fillRect(150, (var1 - 5), 479, (var1 + 25));
        myGLCD.setColor(VGA_BLACK);
        sprintf(printString, "%04X  %d  %02X  %02X  %02X  %02X  %02X  %02X  %02X  %02X", rxId, len, rxBuf[0], rxBuf[1], rxBuf[2], rxBuf[3], rxBuf[4], rxBuf[5], rxBuf[6], rxBuf[7]);
        myGLCD.print(printString, 150, var1);

        if (var1 < 300)
        {
            var1 += 15;
        }
        else
        {
            var1 = 60;
        }
    }
    myGLCD.setFont(BigFont);
}

/*============== CAN: Serial ==============*/
void drawCANSerial()
{
    drawSquareBtn(145, 55, 479, 319, "", themeBackground, themeBackground, themeBackground, CENTER);
    drawSquareBtn(145, 140, 479, 160, F("View CAN on serial"), themeBackground, themeBackground, menuBtnColor, CENTER);
}

/*============== Send Frame ==============*/
// Note: var2 used by send frame function and var1 used by keypad function
void drawSendFrame(uint8_t channel)
{
    drawSquareBtn(145, 55, 479, 319, "", themeBackground, themeBackground, themeBackground, CENTER);

    drawRoundBtn(145, 55, 473, 95, F("ID"), menuBackground, menuBtnBorder, menuBtnText, CENTER);
    drawRoundBtn(145, 100, 473, 145, String(can1.getCANOutID(), 16), menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
    drawRoundBtn(145, 150, 473, 190, F("FRAME"), menuBackground, menuBtnBorder, menuBtnText, CENTER);
 
    myGLCD.setFont(SmallFont);
    drawRoundBtn(145, 195, 186, 250, " " + String(can1.getCANOutData(0), 16), menuBtnColor, menuBtnBorder, menuBtnText, LEFT);
    drawRoundBtn(186, 195, 227, 250, " " + String(can1.getCANOutData(1), 16), menuBtnColor, menuBtnBorder, menuBtnText, LEFT);
    drawRoundBtn(227, 195, 268, 250, " " + String(can1.getCANOutData(2), 16), menuBtnColor, menuBtnBorder, menuBtnText, LEFT);
    drawRoundBtn(268, 195, 309, 250, " " + String(can1.getCANOutData(3), 16), menuBtnColor, menuBtnBorder, menuBtnText, LEFT);
    drawRoundBtn(309, 195, 350, 250, " " + String(can1.getCANOutData(4), 16), menuBtnColor, menuBtnBorder, menuBtnText, LEFT);
    drawRoundBtn(350, 195, 391, 250, " " + String(can1.getCANOutData(5), 16), menuBtnColor, menuBtnBorder, menuBtnText, LEFT);
    drawRoundBtn(391, 195, 432, 250, " " + String(can1.getCANOutData(6), 16), menuBtnColor, menuBtnBorder, menuBtnText, LEFT);
    drawRoundBtn(432, 195, 473, 250, " " + String(can1.getCANOutData(7), 16), menuBtnColor, menuBtnBorder, menuBtnText, LEFT);
    myGLCD.setFont(BigFont);

    drawRoundBtn(145, 255, 473, 300, F("Send"), menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
    drawSquareBtn(150, 301, 479, 319, VERSION, themeBackground, themeBackground, menuBtnColor, CENTER);
}

void sendFrameButtons(uint8_t channel)
{
    // Touch screen controls
    if (myTouch.dataAvailable())
    {
        myTouch.read();
        x = myTouch.getX();
        y = myTouch.getY();

        if ((x >= 145) && (x <= 473))
        {
            if ((y >= 100) && (y <= 145))
            {
                waitForIt(145, 100, 473, 145);
                // Set ID
                state = 1;
                isFinished = false;
            }
        }
        if ((y >= 195) && (y <= 250))
        {
            if ((x >= 145) && (x <= 186))
            {
                waitForIt(145, 195, 186, 250);
                // Set Data[0]
                state = 2;
                isFinished = false;
            }
            if ((x >= 186) && (x <= 227))
            {
                waitForIt(186, 195, 227, 250);
                // Set Data[1]
                state = 3;
                isFinished = false;
            }
            if ((x >= 227) && (x <= 268))
            {
                waitForIt(227, 195, 268, 250);
                // Set Data[2]
                state = 4;
                isFinished = false;
            }
            if ((x >= 268) && (x <= 309))
            {
                waitForIt(268, 195, 309, 250);
                // Set Data[3]
                state = 5;
                isFinished = false;
            }
            if ((x >= 309) && (x <= 350))
            {
                waitForIt(309, 195, 350, 250);
                // Set Data[4]
                state = 6;
                isFinished = false;
            }
            if ((x >= 350) && (x <= 391))
            {
                waitForIt(350, 195, 391, 250);
                // Set Data[5]
                state = 7;
                isFinished = false;
            }
            if ((x >= 391) && (x <= 432))
            {
                waitForIt(391, 195, 432, 250);
                // Set Data[6]
                state = 8;
                isFinished = false;
            }
            if ((x >= 432) && (x <= 473))
            {
                waitForIt(432, 195, 473, 250);
                // Set Data[7]
                state = 9;
                isFinished = false;
            }
        }
        if ((x >= 145) && (x <= 473))
        {
            if ((y >= 255) && (y <= 300))
            {
                waitForIt(145, 255, 473, 300);
                // Send Frame
                can1.sendCANOut(channel, selectedChannelOut);
            }
        }
    }
}

// Using this function in sendFrame switch statement to avoid writing it out 8 times
void setData(uint8_t position)
{
    char displayText[10];
    if (!isFinished)
    {
        drawKeypad();
        sprintf(displayText, "%02X", can1.getCANOutData(position));
        isFinished = true;
        counter1 = 1;
        var2 = 0xFF;
        var3 = 0;
        drawRoundBtn(145, 220, 470, 260, displayText, menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
    }

    var2 = keypadButtons();
    if (var2 >= 0x00 && var2 < 0x10 && counter1 >= 0)
    {
        // Var3 = current value + returned keypad value times its hex place
        var3 = var3 + (var2 * hexTable[counter1]);
        sprintf(displayText, "%02X", var3);
        drawRoundBtn(145, 220, 470, 260, displayText, menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
        if (counter1 >= 0)
        {
            counter1--;
        }
        var2 = 0xFF;
    }
    if (var2 == 0x10)
    {
        counter1 = 2;
        var2 = 0;
        var3 = 0;
        drawRoundBtn(145, 220, 470, 260, F("0"), menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
    }
    if (var2 == 0x11)
    {
        can1.setDataCANOut(var3, position);
        isFinished = false;
        state = 0;
    }
    if (var2 == 0x12)
    {
        isFinished = false;
        state = 0;
    }
}
 
//
void sendFrame(uint8_t channel)
{
    char displayText[10];
    switch (state)
    {
        case 0: // 
            if (!isFinished)
            {
                drawSendFrame(1);
                isFinished = true;
            }
            sendFrameButtons(channel);
            break;
        case 1: // ID
            if (!isFinished)
            {
                drawKeypad();
                sprintf(displayText, "%03X", can1.getCANOutID());
                isFinished = true;
                counter1 = 2;
                var2 = 0xFF;
                var4 = 0;
                drawRoundBtn(145, 220, 470, 260, displayText, menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
            }

            var2 = keypadButtons();
            if (var2 >= 0x00 && var2 < 0x10 && counter1 >= 0)
            {
                // Var3 = current value + returned keypad value times its hex place
                var4 = var4 + (var2 * hexTable[counter1]);
                sprintf(displayText, "%03X", var4);
                drawRoundBtn(145, 220, 470, 260, displayText, menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
                if (counter1 >= 0)
                {
                    counter1--;
                }
                var2 = 0xFF;
            }
            if (var2 == 0x10)
            {
                counter1 = 2;
                var2 = 0;
                var4 = 0;
                drawRoundBtn(145, 220, 470, 260, F("0"), menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
            }
            if (var2 == 0x11)
            {
                can1.setIDCANOut(var4);
                drawRoundBtn(145, 100, 473, 145, String(can1.getCANOutID(), 16), menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
                isFinished = false;
                state = 0;
            }
            if (var2 == 0x12)
            {
                isFinished = false;
                state = 0;
            }
            break;
        case 2: setData(0);
            break;
        case 3: setData(1); 
            break;
        case 4: setData(2);
            break;
        case 5: setData(3);
            break;
        case 6: setData(4);
            break;
        case 7: setData(5);
            break;
        case 8: setData(6);
            break;
        case 9: setData(7);
            break;
    }
}


/*=========================================================
    Vehicle Tools
===========================================================*/
void drawVehicleTools()
{
    switch (graphicLoaderState)
    {
    case 0:
        break;
    case 1:
        drawSquareBtn(131, 55, 479, 319, "", themeBackground, themeBackground, themeBackground, CENTER);
        break;
    case 2:
        drawRoundBtn(145, 80, 308, 130, F("PIDSCAN"), menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
        break;
    case 3:
        drawRoundBtn(312, 80, 475, 130, F("PIDSTRM"), menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
        break;
    case 4:
        drawRoundBtn(145, 135, 308, 185, F("PID Guages"), menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
        break;
    case 5:
        drawRoundBtn(312, 135, 475, 185, F("VIN"), menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
        break;
    case 6:
        drawRoundBtn(145, 190, 308, 240, F("Scan DTC"), menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
        break;
    case 7:
        drawRoundBtn(312, 190, 475, 240, F("Clear DTC"), menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
        break;
    case 8:
        //drawRoundBtn(145, 245, 308, 295, F("Unused"), menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
        break;
    case 9:
        //drawRoundBtn(312, 245, 475, 295, F("Unused"), menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
        break;
    case 10:
        drawSquareBtn(150, 300, 479, 319, VERSION, themeBackground, themeBackground, menuBtnColor, CENTER);
        break;
    } 
}

// TODO: Odd even button page assignments are swapped
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

                // Initialize state machine variables to 0
                state = 0;
                isFinished = false;
                nextState = true;
                timer1 = 0;
                counter1 = 0;
                var1 = 0;
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
                state = 0;
                var1 = 0;
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

                // Initialize state machine variables to 0
                state = 0;
                counter1 = 0;
                timer1 = 0;
                isFinished = false;
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
    drawSquareBtn(145, 55, 479, 319, "", themeBackground, themeBackground, themeBackground, CENTER);
    drawSquareBtn(141, 90, 479, 110, F("Scan supported PIDs"), themeBackground, themeBackground, menuBtnColor, CENTER);
    drawSquareBtn(141, 115, 479, 135, F("to SD Card"), themeBackground, themeBackground, menuBtnColor, CENTER);
}

void startPIDSCAN()
{
    // Run once at start
    if (( millis() - timer1  > 200 ) && !isFinished )
    {
        loadBar(state);

        // Get vehicle vin, will be saved in can1 object
        const uint16_t rxid = 0x7E8;
        can1.requestVIN(rxid, true);

        loadBar(state++);
    }

    // Cycle though all available banks of PIDS
    if (nextState && ( millis() - timer1 >= 100 ))
    {
        // Get PID list with current range and bank
        can1.getPIDList(var1, counter1);
        var1 = var1 + 0x20;
        counter1++;

        loadBar(state++);

        // Check last bit to see if there are more PIDs in the next bank
        nextState = can1.getNextPID();

        timer1 = millis();
    }

    // Finished
    if (!nextState && !isFinished)
    {
        loadBar(DONE);

        // Activate the PIDSTRM page 
        hasPID = true;

        isFinished = true;
    }
}

/*========== PID Stream Functions ==========*/
void drawPIDStreamScroll()
{
    // Temp to hold PIDS value before strcat
    char temp[2];

    // Starting y location for list
    uint16_t y = 60;

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

void drawPIDStream()
{
    drawSquareBtn(141, 55, 479, 319, "", themeBackground, themeBackground, themeBackground, CENTER);
    myGLCD.setColor(menuBtnColor);
    myGLCD.setBackColor(themeBackground);
    drawSquareBtn(420, 80, 470, 160, F("/\\"), menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
    drawSquareBtn(420, 160, 470, 240, F("\\/"), menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
    drawPIDStreamScroll();
    drawRoundBtn(150, 275, 410, 315, F("Stream PID"), menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
}

//
void PIDStreamButtons()
{
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
                var1 = 1 + scroll;
            }
            if ((y >= 95) && (y <= 130))
            {
                waitForItRect(150, 95, 410, 130);
                Serial.println(2 + scroll);
                var1 = 2 + scroll;
            }
            if ((y >= 130) && (y <= 165))
            {
                waitForItRect(150, 130, 410, 165);
                Serial.println(3 + scroll);
                var1 = 3 + scroll;
            }
            if ((y >= 165) && (y <= 200))
            {
                waitForItRect(150, 165, 410, 200);
                Serial.println(4 + scroll);
                var1 = 4 + scroll;
            }
            if ((y >= 200) && (y <= 235))
            {
                waitForItRect(150, 200, 410, 235);
                Serial.println(5 + scroll);
                var1 = 5 + scroll;
            }
            if ((y >= 235) && (y <= 270))
            {
                waitForItRect(150, 235, 410, 270);
                Serial.println(6 + scroll);
                var1 = 6 + scroll;
            }
        }
        if ((x >= 420) && (x <= 470))
        {
            if ((y >= 80) && (y <= 160))
            {
                waitForItRect(420, 80, 470, 160);
                if (scroll > 0)
                {
                    scroll = scroll - 6;
                    drawPIDStreamScroll();
                }
            }
        }
        if ((x >= 420) && (x <= 470))
        {
            if ((y >= 160) && (y <= 240))
            {
                waitForItRect(420, 160, 470, 240);
                if (scroll < 100)
                {
                    scroll = scroll + 6;
                    drawPIDStreamScroll();
                }
            }
        }
        if ((x >= 150) && (x <= 410))
        {
            if ((y >= 275) && (y <= 315))
            {
                if (var1 != 0)
                {
                    waitForItRect(150, 275, 410, 315);
                    Serial.print("Sending PID: ");
                    Serial.println(arrayIn[var1 + 1]);
                    state = 1;
                    counter1 = 0;
                    timer1 = 0;
                }
            }
        }
    }
}

/*============== PID Stream Guages ==============*/
void PIDGauges()
{
    bool isWait = true;

    drawSquareBtn(145, 60, 479, 319, "", themeBackground, themeBackground, themeBackground, CENTER);

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
    myGLCD.print(F("Load"), 188, 148); // 4
    myGLCD.print(F("RPM"), 358, 148);// C
    myGLCD.print(F("TEMP"), 188, 278); // 5
    myGLCD.print(F("MPH"), 358, 278); // D
    myGLCD.setBackColor(menuBtnColor);

    const float pi = 3.14159;
    const uint8_t r = 50;

    uint16_t x1 = r * cos(pi / 2) + 220;
    uint16_t y1 = r * sin(pi / 2) + 120;
    uint16_t x2 = r * cos(pi / 2) + 380;
    uint16_t y2 = r * sin(pi / 2) + 120;
    uint16_t x3 = r * cos(pi / 2) + 220;
    uint16_t y3 = r * sin(pi / 2) + 250;
    uint16_t x4 = r * cos(pi / 2) + 380;
    uint16_t y4 = r * sin(pi / 2) + 250;

    const float offset = (pi / 2) + 1;
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
        g1 = can1.PIDStream(CAN_PID_ID, 0x4, false);
        g2 = can1.PIDStream(CAN_PID_ID, 0xC, false);
        g3 = can1.PIDStream(CAN_PID_ID, 0x5, false);
        g4 = can1.PIDStream(CAN_PID_ID, 0xD, false);
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
    const uint16_t rxid = 0x7E8;
    drawSquareBtn(145, 55, 479, 319, "", themeBackground, themeBackground, themeBackground, CENTER);
    can1.requestVIN(rxid, false);
    drawSquareBtn(150, 150, 479, 170, F("VIN"), themeBackground, themeBackground, menuBtnColor, CENTER);
    drawSquareBtn(150, 180, 479, 200, can1.getVIN(), themeBackground, themeBackground, menuBtnColor, CENTER);
}

/*============== DTC SCAN ==============*/
// Draw
// Function

// Buttons

/*============== DTC CLEAR ==============*/
void clearDTC()
{
    // Run once at start
    if (millis() - timer1 > 500 && !isFinished)
    {
        drawSquareBtn(145, 60, 479, 319, "", themeBackground, themeBackground, themeBackground, CENTER);
        drawSquareBtn(150, 150, 479, 170, F("Clearing DTCS..."), themeBackground, themeBackground, menuBtnColor, CENTER);
    }

    // Cycle through clear messages
    if (state < 7)
    {
        if (millis() - timer1 >= 400)
        {
            uint32_t IDc[7] = { 0x7D0, 0x720, 0x765, 0x737, 0x736, 0x721, 0x760 };
            byte MSGc[8] = { 0x4, 0x18, 0x00, 0xFF, 0x00, 0x55, 0x55, 0x55 };

            can1.sendFrame(IDc[state], MSGc, 8, selectedChannelOut);
            counter1++;
            timer1 = millis();
        }
        if (counter1 == 3)
        {
            counter1 = 0;
            loadBar(1 + state++);
        }
    }

    // Finished
    if (state > 6 && !isFinished)
    {
        drawSquareBtn(150, 150, 479, 170, F("All DTCS Cleared"), themeBackground, themeBackground, menuBtnColor, CENTER);
        isFinished = true;
        loadBar(DONE);
    }
}


/*=========================================================
    Polaris Tools
===========================================================*/
void drawRZRTOOL()
{
    switch (graphicLoaderState)
    {
    case 0:
        break;
    case 1:
        drawSquareBtn(131, 55, 479, 319, "", themeBackground, themeBackground, themeBackground, CENTER);
        break;
    case 2:
        drawRoundBtn(145, 80, 308, 130, F("Scan DTC"), menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
        break;
    case 3:
        drawRoundBtn(312, 80, 475, 130, F("Clear DTC"), menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
        break;
    case 4:
        //drawRoundBtn(145, 135, 308, 185, F("Unused"), menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
        break;
    case 5:
        //drawRoundBtn(312, 135, 475, 185, F("Unused"), menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
        break;
    case 6:
        //drawRoundBtn(145, 190, 308, 240, F("Unused"), menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
        break;
    case 7:
        //drawRoundBtn(312, 190, 475, 240, F("Unused"), menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
        break;
    case 8:
        //drawRoundBtn(145, 245, 308, 295, F("Unused"), menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
        break;
    case 9:
        //drawRoundBtn(312, 245, 475, 295, F("Unused"), menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
        break;
    case 10:
        drawSquareBtn(150, 300, 479, 319, VERSION, themeBackground, themeBackground, menuBtnColor, CENTER);
        break;
    }  
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
    switch (graphicLoaderState)
    {
    case 0:
        break;
    case 1:
        drawSquareBtn(131, 55, 479, 319, "", themeBackground, themeBackground, themeBackground, CENTER);
        break;
    case 2:
        drawRoundBtn(145, 80, 308, 130, F("CAN Cap"), menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
        break;
    case 3:
        //drawRoundBtn(312, 80, 475, 130, F("Unused"), menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
        break;
    case 4:
        //drawRoundBtn(145, 135, 308, 185, F("Unused"), menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
        break;
    case 5:
        //drawRoundBtn(312, 135, 475, 185, F("Unused"), menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
        break;
    case 6:
        //drawRoundBtn(145, 190, 308, 240, F("Unused"), menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
        break;
    case 7:
        drawRoundBtn(312, 190, 475, 240, F("Jeep Auto"), menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
        break;
    case 8:
        //drawRoundBtn(145, 245, 308, 295, F("Unused"), menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
        break;
    case 9:
        drawRoundBtn(312, 245, 475, 295, F("Dongle"), menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
        break;
    case 10:
        drawSquareBtn(150, 300, 479, 319, VERSION, themeBackground, themeBackground, menuBtnColor, CENTER);
        break;
    }
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
                // CAN capture playback
                page = 28;
                hasDrawn = false;
            }
            if ((y >= 135) && (y <= 185))
            {
                //waitForIt(145, 135, 308, 185);
                // Unused
                //page = 30;
                //hasDrawn = false;
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
                //waitForIt(312, 80, 475, 130);
                // Unused
                //page = 29;
                //hasDrawn = false;
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
                waitForIt(312, 190, 475, 240);
                // Jeep Auto Start/Stop
                page = 33;
                hasDrawn = false;
            }
            if ((y >= 245) && (y <= 295))
            {
                waitForIt(312, 245, 475, 295);
                // GM Dongle Sim
                page = 35;
                hasDrawn = false;
                graphicLoaderState = 0;
            }
        }
    }
}

void drawCANLog()
{
    // clear LCD
    drawSquareBtn(131, 55, 479, 319, "", themeBackground, themeBackground, themeBackground, CENTER);

    drawSquareBtn(420, 80, 470, 160, F("/\\"), menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
    drawSquareBtn(420, 160, 470, 240, F("\\/"), menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
    drawSquareBtn(131, 275, 216, 315, F("Play"), menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
    drawSquareBtn(216, 275, 301, 315, F("Del"), menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
    drawSquareBtn(301, 275, 386, 315, F("Split"), menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
    drawSquareBtn(386, 275, 479, 315, F("Conf"), menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
}

//
void drawCANLogScroll()
{
    isWaitForIt = false;
    File root1;
    root1 = SD.open("/");
  
    uint8_t size = sdCard.printDirectory(root1, fileList);
  
    // Starting y location for list
    uint16_t y = 60;

    // Draw the scroll window
    for (int i = 0; i < MAXSCROLL; i++)
    {
        if ((scroll + i < 10))
        {
            char temp[13];
            sprintf(temp, "%s", fileList[scroll + i]);
            drawSquareBtn(150, y, 410, y + 35, temp, menuBackground, menuBtnBorder, menuBtnText, LEFT);
        }
        else
        {
            drawSquareBtn(150, y, 410, y + 35, "", menuBackground, menuBtnBorder, menuBtnText, LEFT);
        }
        y = y + 35;
    }
}

//
void CANLogButtons()
{
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
                var1 = 1 + scroll;
            }
            if ((y >= 95) && (y <= 130))
            {
                waitForItRect(150, 95, 410, 130);
                Serial.println(2 + scroll);
                var1 = 2 + scroll;
            }
            if ((y >= 130) && (y <= 165))
            {
                waitForItRect(150, 130, 410, 165);
                Serial.println(3 + scroll);
                var1 = 3 + scroll;
            }
            if ((y >= 165) && (y <= 200))
            {
                waitForItRect(150, 165, 410, 200);
                Serial.println(4 + scroll);
                var1 = 4 + scroll;
            }
            if ((y >= 200) && (y <= 235))
            {
                waitForItRect(150, 200, 410, 235);
                Serial.println(5 + scroll);
                var1 = 5 + scroll;
            }
            if ((y >= 235) && (y <= 270))
            {
                waitForItRect(150, 235, 410, 270);
                Serial.println(6 + scroll);
                var1 = 6 + scroll;
            }
        }
        if ((x >= 420) && (x <= 470))
        {
            if ((y >= 80) && (y <= 160))
            {
                waitForItRect(420, 80, 470, 160);
                if (scroll > 0)
                {
                    scroll--;
                    drawCANLogScroll();
                }
            }
        }
        if ((x >= 420) && (x <= 470))
        {
            if ((y >= 160) && (y <= 240))
            {
                waitForItRect(420, 160, 470, 240);
                if (scroll < 100)
                {
                    scroll++;
                    drawCANLogScroll();
                }
            }
        }
        if ((y >= 275) && (y <= 315))
        {
            char fileLoc[20] = "CANLOG/";
            strcat(fileLoc, fileList[var1 - 1]);
            if ((x >= 131) && (x <= 216))
            {
                // Play
                waitForItRect(131, 275, 216, 315);
                sdCard.readLogFile(fileLoc);
            }
            if ((x >= 216) && (x <= 301))
            {
                // Delete
                waitForItRect(216, 275, 301, 315);
                drawErrorMSG(F("Confirmation"), F("Permenently"), F("Delete File?"));
                //sdCard.deleteFile(fileLoc);
            }
            if ((x >= 301) && (x <= 386))
            {
                // Split
                waitForItRect(301, 275, 386, 315);
                uint32_t temp = sdCard.fileLength(fileLoc);
                sdCard.tempCopy(fileLoc);
                sdCard.split("canlog/temp.txt", temp);
                sdCard.deleteFile("canlog/temp.txt");
            }
            if ((x >= 386) && (x <= 479))
            {
                // Settings
                waitForItRect(386, 275, 479, 315);
                sdCard.readLogFile(fileList[var1]);
            }
        }
 
    }
}

//
void consoleTextJeep()
{
    const uint16_t id = 0x328;
    uint8_t data1[8] = { 0x40, 0x41, 0x00, 0x4c, 0x00, 0x69, 0x00, 0x67 };
    uint8_t data2[8] = { 0x30, 0x01, 0x00, 0x68, 0x00, 0x74, 0x00, 0x53 };
    uint8_t data3[8] = { 0x30, 0x01, 0x00, 0x68, 0x00, 0x74, 0x00, 0x53 };
    uint8_t data4[8] = { 0x10, 0x01, 0x00, 0x3f, 0x00, 0x00, 0x00, 0x00 };
    uint8_t data5[8] = { 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

    can1.sendFrame(id, data1, 8, selectedChannelOut);
    can1.sendFrame(id, data2, 8, selectedChannelOut);
    can1.sendFrame(id, data3, 8, selectedChannelOut);
    can1.sendFrame(id, data4, 8, selectedChannelOut);
    can1.sendFrame(id, data5, 8, selectedChannelOut);
}

//
void disableAutoStopJeep()
{
    const uint16_t id = 0x137;
    uint8_t data1[8] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x0d };
    can1.sendFrame(id, data1, 8, selectedChannelOut);
}

//
void drawDongleSim()
{
    switch (graphicLoaderState)
    {
    case 0:
        drawSquareBtn(145, 55, 479, 319, "", themeBackground, themeBackground, themeBackground, CENTER);
        break;
    case 1:
        drawRoundBtn(145, 55, 250, 100, F("VIN"), menuBackground, menuBtnBorder, menuBtnText, CENTER);
        break;
    case 2:
        drawRoundBtn(255, 55, 325, 100, F("G"), menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
        break;
    case 3:
        drawRoundBtn(330, 55, 400, 100, F("F"), menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
        break;
    case 4:
        drawRoundBtn(405, 55, 475, 100, F("C"), menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
        break;
    case 5:
        drawRoundBtn(145, 105, 250, 150, F("Eng G"), menuBackground, menuBtnBorder, menuBtnText, CENTER);
        break;
    case 6:
        drawRoundBtn(255, 105, 325, 150, F("Off"), menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
        break;
    case 7:
        drawRoundBtn(330, 105, 400, 150, F("Acc"), menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
        break;
    case 8:
        drawRoundBtn(405, 105, 475, 150, F("On"), menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
        break;
    case 9:
        drawRoundBtn(145, 155, 250, 200, F("ECU"), menuBackground, menuBtnBorder, menuBtnText, CENTER);
        break;
    case 10:
        drawRoundBtn(255, 155, 362, 200, F("Accept"), menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
        break;
    case 11:
        drawRoundBtn(368, 155, 475, 200, F("Reject"), menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
        break;
    case 12:
        drawRoundBtn(145, 205, 250, 250, F("Eng F"), menuBackground, menuBtnBorder, menuBtnText, CENTER);
        break;
    case 13:
        drawRoundBtn(255, 205, 325, 250, F("0"), menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
        break;
    case 14:
        drawRoundBtn(330, 205, 400, 250, F("2"), menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
        break;
    case 15:
        drawRoundBtn(405, 205, 475, 250, F("1"), menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
        break;
    case 16 :
        drawRoundBtn(145, 255, 250, 300, F("Aut F"), menuBackground, menuBtnBorder, menuBtnText, CENTER);
        break;
    case 17:
        drawRoundBtn(255, 255, 362, 300, F("opt1"), menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
        break;
    case 18:
        drawRoundBtn(368, 255, 475, 300, F("opt2"), menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
        break;
    case 19:
        drawSquareBtn(150, 301, 479, 319, VERSION, themeBackground, themeBackground, menuBtnColor, CENTER);
        break;
    } 
}

// 
void dongleSimButtons()
{
     // Touch screen controls
    if (myTouch.dataAvailable())
    {
        myTouch.read();
        x = myTouch.getX();
        y = myTouch.getY();

        // VIN
        if ((y >= 55) && (y <= 100) && millis() - timer2 > 20)
        {
            if ((x >= 255) && (x <= 325))
            {
                waitForIt(255, 55, 325, 100);
                // 1
                const uint16_t id = 0x7E8;
                uint8_t data1[8] = { 0x10, 0x00, 0x00, 0x00, 0x00, 0x31, 0x47, 0x54 };
                can1.sendFrame(id, data1, 8, selectedChannelOut);
                delay(10);
                serialOut();
                uint8_t data2[8] = { 0x21, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37 };
                can1.sendFrame(id, data2, 8, selectedChannelOut);
                delay(5);
                serialOut();
                uint8_t data3[8] = { 0x22, 0x38, 0x39, 0x41, 0x42, 0x43, 0x44, 0x45 };
                can1.sendFrame(id, data3, 8, selectedChannelOut);
            }
            if ((x >= 330) && (x <= 400))
            {
                waitForIt(255, 55, 325, 100);
                // 1
                const uint16_t id = 0x7E8;
                uint8_t data1[8] = { 0x10, 0x00, 0x00, 0x00, 0x00, 0x31, 0x46, 0x54 };
                can1.sendFrame(id, data1, 8, selectedChannelOut);
                delay(10);
                serialOut();
                uint8_t data2[8] = { 0x21, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37 };
                can1.sendFrame(id, data2, 8, selectedChannelOut);
                delay(5);
                serialOut();
                uint8_t data3[8] = { 0x22, 0x38, 0x39, 0x41, 0x42, 0x43, 0x44, 0x45 };
                can1.sendFrame(id, data3, 8, selectedChannelOut);
            }
            if ((x >= 405) && (x <= 475))
            {
                waitForIt(255, 55, 325, 100);
                // 1
                const uint16_t id = 0x7E8;
                uint8_t data1[8] = { 0x10, 0x00, 0x00, 0x00, 0x00, 0x31, 0x43, 0x54 };
                can1.sendFrame(id, data1, 8, selectedChannelOut);
                delay(10);
                serialOut();
                uint8_t data2[8] = { 0x21, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37 };
                can1.sendFrame(id, data2, 8, selectedChannelOut);
                delay(5);
                serialOut();
                uint8_t data3[8] = { 0x22, 0x38, 0x39, 0x41, 0x42, 0x43, 0x44, 0x45 };
                can1.sendFrame(id, data3, 8, selectedChannelOut);
            }
            timer2 = millis();
        }

        // Engine GM
        if ((y >= 105) && (y <= 150) && millis() - timer2 > 20)
        {
            if ((x >= 255) && (x <= 325))
            {
                waitForIt(255, 105, 325, 150);
                // Off
                const uint16_t id = 0x641;
                uint8_t data[8] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
                can1.sendFrame(id, data, 8, selectedChannelOut);
            }
            if ((x >= 330) && (x <= 400))
            {
                waitForIt(330, 105, 400, 150);
                // Acc
                const uint16_t id = 0x641;
                uint8_t data[8] = { 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00 };
                can1.sendFrame(id, data, 8, selectedChannelOut);
            }
            if ((x >= 405) && (x <= 475))
            {
                waitForIt(405, 105, 475, 150);
                // On
                const uint16_t id = 0x641;
                uint8_t data[8] = { 0x00, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00 };
                can1.sendFrame(id, data, 8, selectedChannelOut);
            }
            timer2 = millis();
        }

        // ECU GM
        if ((y >= 155) && (y <= 200) && millis() - timer2 > 20)
        {
            if ((x >= 255) && (x <= 362))
            {
                waitForIt(255, 155, 362, 200);
                // Accept
                const uint16_t id = 0x7E8;
                uint8_t data[8] = { 0x02, 0x60, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
                can1.sendFrame(id, data, 8, selectedChannelOut);
            }
            if ((x >= 368) && (x <= 475))
            {
                waitForIt(368, 155, 475, 200);
                // Reject
                const uint16_t id = 0x7E8;
                uint8_t data[8] = { 0x02, 0x7F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
                can1.sendFrame(id, data, 8, selectedChannelOut);
            }
            timer2 = millis();
        }

        // Engine Ford
        if ((y >= 205) && (y <= 250) && millis() - timer2 > 20)
        {
            const uint16_t id = 0x42F;
            if ((x >= 255) && (x <= 325))
            {
                waitForIt(255, 205, 325, 250);
                uint8_t data[8] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
                can1.sendFrame(id, data, 8, selectedChannelOut);
            }
            if ((x >= 330) && (x <= 400))
            {
                waitForIt(330, 205, 400, 250);
                uint8_t data[8] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x87 };
                can1.sendFrame(id, data, 8, selectedChannelOut);
            }
            if ((x >= 405) && (x <= 475))
            {
                waitForIt(405, 205, 475, 250);
                uint8_t data[8] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0xEA };
                can1.sendFrame(id, data, 8, selectedChannelOut);
            }
            timer2 = millis();
        }

        // Auto Ford
        if ((y >= 255) && (y <= 300) && millis() - timer2 > 20)
        {
            const uint16_t id = 0x326;
            if ((x >= 255) && (x <= 362))
            {
                waitForIt(255, 255, 362, 300);
                uint8_t data[8] = { 0x81, 0xA0, 0x25, 0xB5, 0x81, 0xA0, 0x00, 0x00 };
                can1.sendFrame(id, data, 8, selectedChannelOut);
            }
            if ((x >= 368) && (x <= 475))
            {
                waitForIt(368, 255, 475, 300);
                uint8_t data[8] = { 0x01, 0xA0, 0x45, 0xBA, 0x81, 0x90, 0x00, 0x00 };
                can1.sendFrame(id, data, 8, selectedChannelOut);
            }
            timer2 = millis();
        }
    }
}

void dongleSimButtonsFord()
{
    // Touch screen controls
    if (myTouch.dataAvailable())
    {
        myTouch.read();
        x = myTouch.getX();
        y = myTouch.getY();

        // VIN G
        // 255, 65, 325, 100 
        // 330, 65, 400, 100
        // 405, 65, 475, 100
        if ((y >= 55) && (y <= 100) && millis() - timer2 > 20)
        {
            if ((x >= 255) && (x <= 325))
            {
                waitForIt(255, 55, 325, 100);
                // 1
                const uint16_t id = 0x7E8;
                uint8_t data[8] = { 0x10, 0x00, 0x00, 0x00, 0x00, 0x31, 0x46, 0x54 };
                can1.sendFrame(id, data, 8, selectedChannelOut);
            }
            if ((x >= 330) && (x <= 400))
            {
                waitForIt(330, 55, 400, 100);
                // 2
                const uint16_t id = 0x7E8;
                uint8_t data[8] = { 0x21, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37 };
                can1.sendFrame(id, data, 8, selectedChannelOut);
            }
            if ((x >= 405) && (x <= 475))
            {
                waitForIt(405, 55, 475, 100);
                // 3
                const uint16_t id = 0x7E8;
                uint8_t data[8] = { 0x22, 0x38, 0x39, 0x41, 0x42, 0x43, 0x44, 0x45 };
                can1.sendFrame(id, data, 8, selectedChannelOut);
            }
            timer2 = millis();
        }

        // Engine
        // 255, 105, 325, 140
        // 330, 105, 400, 140
        // 405, 105, 475, 140
        if ((y >= 105) && (y <= 150) && millis() - timer2 > 20)
        {
            if ((x >= 255) && (x <= 325))
            {
                waitForIt(255, 105, 325, 150);
                // Off
                const uint16_t id = 0x641;
                uint8_t data[8] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
                can1.sendFrame(id, data, 8, selectedChannelOut);
            }
            if ((x >= 330) && (x <= 400))
            {
                waitForIt(330, 105, 400, 150);
                // Acc
                const uint16_t id = 0x641;
                uint8_t data[8] = { 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00 };
                can1.sendFrame(id, data, 8, selectedChannelOut);
            }
            if ((x >= 405) && (x <= 475))
            {
                waitForIt(405, 105, 475, 150);
                // On
                const uint16_t id = 0x641;
                uint8_t data[8] = { 0x00, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00 };
                can1.sendFrame(id, data, 8, selectedChannelOut);
            }
            timer2 = millis();
        }

        // ECU
        // 255, 145, 362, 180
        // 368, 145, 475, 180
        if ((y >= 155) && (y <= 200) && millis() - timer2 > 20)
        {
            if ((x >= 255) && (x <= 362))
            {
                waitForIt(255, 155, 362, 200);
                // Accept
                const uint16_t id = 0x7E8;
                uint8_t data[8] = { 0x02, 0x60, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
                can1.sendFrame(id, data, 8, selectedChannelOut);
            }
            if ((x >= 368) && (x <= 475))
            {
                waitForIt(368, 155, 475, 200);
                // Reject
                const uint16_t id = 0x7E8;
                uint8_t data[8] = { 0x02, 0x7F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
                can1.sendFrame(id, data, 8, selectedChannelOut);
            }
            timer2 = millis();
        }

        // VIN F
        // 255, 185, 325, 220
        // 330, 185, 400, 220
        // 405, 185, 475, 220
        if ((y >= 205) && (y <= 250) && millis() - timer2 > 20)
        {
            if ((x >= 255) && (x <= 325))
            {
                waitForIt(255, 205, 325, 250);
                // 10
                const uint16_t id = 0x7E8;
                uint8_t data[8] = { 0x10, 0x00, 0x00, 0x00, 0x00, 0x31, 0x46, 0x54 };
                can1.sendFrame(id, data, 8, selectedChannelOut);
            }
            if ((x >= 330) && (x <= 400))
            {
                waitForIt(330, 205, 400, 250);
                // 20
                const uint16_t id = 0x7E8;
                uint8_t data[8] = { 0x21, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37 };
                can1.sendFrame(id, data, 8, selectedChannelOut);
            }
            if ((x >= 405) && (x <= 475))
            {
                waitForIt(405, 205, 475, 250);
                // 30
                const uint16_t id = 0x7E8;
                uint8_t data[8] = { 0x22, 0x38, 0x39, 0x41, 0x42, 0x43, 0x44, 0x45 };
                can1.sendFrame(id, data, 8, selectedChannelOut);
            }
            timer2 = millis();
        }

        // VIN C
        // 255, 225, 325, 260
        // 330, 225, 400, 260
        // 405, 225, 475, 260
        if ((y >= 255) && (y <= 300) && millis() - timer2 > 20)
        {
            if ((x >= 255) && (x <= 325))
            {
                waitForIt(255, 255, 325, 300);
                // 0x01
                const uint16_t id = 0x7E8;
                uint8_t data[8] = { 0x04, 0x41, 0x0C, 0x40, 0x00, 0x00, 0x00, 0x00 };
                can1.sendFrame(id, data, 8, selectedChannelOut);
            }
            if ((x >= 330) && (x <= 400))
            {
                waitForIt(330, 255, 400, 300);
                // 0x02
                const uint16_t id = 0x7E8;
                uint8_t data[8] = { 0x04, 0x41, 0x0C, 0x00, 0x00, 0x00, 0x00, 0x00 };
                can1.sendFrame(id, data, 8, selectedChannelOut);
            }
            if ((x >= 405) && (x <= 475))
            {
                waitForIt(405, 255, 475, 300);
                // 0x08
                const uint16_t id = 0x7E8;
                uint8_t data[8] = { 0x22, 0x38, 0x39, 0x41, 0x42, 0x43, 0x44, 0x45 };
                can1.sendFrame(id, data, 8, selectedChannelOut);
            }
            timer2 = millis();
        }
    }
}


/*=========================================================
    Settings
===========================================================*/
void drawSettings()
{
    switch (graphicLoaderState)
    {
    case 0:
        break;
    case 1:
        drawSquareBtn(131, 55, 479, 319, "", themeBackground, themeBackground, themeBackground, CENTER);
        break;
    case 2:
        drawRoundBtn(145, 80, 308, 130, F("FilterMask"), menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
        break;
    case 3:
        //drawRoundBtn(312, 80, 475, 130, F("Unused"), menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
        break;
    case 4:
        drawRoundBtn(145, 135, 308, 185, F("Set Baud0"), menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
        break;
    case 5:
        drawRoundBtn(312, 135, 475, 185, F("Set Baud1"), menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
        break;
    case 6:
        drawRoundBtn(145, 190, 308, 240, F("Serial Off"), menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
        break;
    case 7:
        //drawRoundBtn(312, 190, 475, 240, F("Unused"), menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
        break;
    case 8:
        //drawRoundBtn(145, 245, 308, 295, F("Unused"), menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
        break;
    case 9:
        //drawRoundBtn(312, 245, 475, 295, F("Unused"), menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
        break;
    case 10:
        drawSquareBtn(150, 300, 479, 319, VERSION, themeBackground, themeBackground, menuBtnColor, CENTER);
        break;
    }
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
                // Filter Mask
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
                waitForIt(145, 190, 308, 240);
                // Serial Off
                page = 41;
                hasDrawn = false;
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
                //waitForIt(312, 80, 475, 130);
                // Unused
                //page = 38;
                //hasDrawn = false;
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
                //Unused
                //page = 44;
                //hasDrawn = false;
            }
        }
    }
}

void drawFilterMask()
{
    drawSquareBtn(145, 55, 479, 319, "", themeBackground, themeBackground, themeBackground, CENTER);

    drawRoundBtn(145, 70, 200, 120, F("CAN"), menuBackground, menuBtnBorder, menuBtnText, CENTER);
    drawRoundBtn(205, 70, 340, 120, F("Filter"), menuBackground, menuBtnBorder, menuBtnText, CENTER);
    drawRoundBtn(345, 70, 475, 120, F("Mask"), menuBackground, menuBtnBorder, menuBtnText, CENTER);

    drawRoundBtn(145, 125, 200, 175, F("0"), menuBackground, menuBtnBorder, menuBtnText, CENTER);
    drawRoundBtn(205, 125, 340, 175, String(CAN0Filter, 16), menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
    drawRoundBtn(345, 125, 475, 175, String(CAN0Mask, 16), menuBtnColor, menuBtnBorder, menuBtnText, CENTER);

    drawRoundBtn(145, 180, 200, 230, F("1"), menuBackground, menuBtnBorder, menuBtnText, CENTER);
    drawRoundBtn(205, 180, 340, 230, String(CAN1Filter, 16), menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
    drawRoundBtn(345, 180, 475, 230, String(CAN1Mask, 16), menuBtnColor, menuBtnBorder, menuBtnText, CENTER);

    drawRoundBtn(145, 235, 475, 285, F("Open All Traffic"), menuBtnColor, menuBtnBorder, menuBtnText, CENTER);

    drawSquareBtn(150, 295, 479, 315, VERSION, themeBackground, themeBackground, menuBtnColor, CENTER);
}

void filterMaskButtons()
{
    // Touch screen controls
    if (myTouch.dataAvailable())
    {
        myTouch.read();
        x = myTouch.getX();
        y = myTouch.getY();

        if ((y >= 125) && (y <= 175))
        {
            if ((x >= 205) && (x <= 340))
            {
                waitForIt(205, 125, 340, 175);
                // Set CAN0 Filter
                state = 1;
                isFinished = false;
            }
            if ((x >= 345) && (x <= 475))
            {
                waitForIt(345, 125, 475, 175);
                // Set CAN 0Mask
                state = 2;
                isFinished = false;
            }
        }
        if ((y >= 180) && (y <= 230))
        {
            if ((x >= 205) && (x <= 340))
            {
                waitForIt(205, 180, 340, 230);
                // Set CAN1 Filter
                state = 3;
                isFinished = false;
            }
            if ((x >= 345) && (x <= 475))
            {
                waitForIt(345, 180, 475, 230);
                // Set CAN1 Mask
                state = 4;
                isFinished = false;
            }
        }
        if ((y >= 235) && (y <= 285))
        {
            if ((x >= 145) && (x <= 475))
            {
                waitForIt(145, 235, 475, 285);
                // Open All Traffic
                can1.startCAN0(0x000, 0x800);
                can1.startCAN1(0x000, 0x800);
            }
        }
    }
}

uint8_t setFilterMask(uint32_t &value)
{
    
    char displayText[10];
    if (!isFinished)
    {
        drawKeypad();
        isFinished = true;
        counter1 = 2;
        var4 =0xFF;
        var5 = 0;
        drawRoundBtn(145, 220, 470, 260, F("000"), menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
    }

    var4 = keypadButtons();
    if (var4 >= 0x00 && var4 < 0x10 && counter1 >= 0)
    {
        // CAN0Filter = current value + returned keypad value times its hex place
        var5 = var5 + (var4 * hexTable[counter1]);
        sprintf(displayText, "%03X", var5);
        drawRoundBtn(145, 220, 470, 260, displayText, menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
        if (counter1 >= 0)
        {
            counter1--;
        }
        var4 = 0xFF;
    }
    if (var4 == 0x10)
    {
        counter1 = 2;
        var4 = 0;
        var5 = 0;
        sprintf(displayText, "%03X", var5);
        drawRoundBtn(145, 220, 470, 260, displayText, menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
    }
    if (var4 == 0x11)
    {
        isFinished = false;
        state = 0;
        value = var5;
        return 0x11;
    }
    if (var4 == 0x12)
    {
        isFinished = false;
        state = 0;
    }
    return 0;
}

void filterMask()
{
    uint8_t temp = 0;
    switch (state)
    {
    case 0: 
        if (!isFinished)
        {
            drawFilterMask();
            isFinished = true;
        }
        filterMaskButtons();
        break;
    case 1: temp = setFilterMask(CAN0Filter);
        if (temp == 0x11)
        {
            can1.setFilterMask0(CAN0Filter, CAN0Mask);
            temp = 0;
        }
        break;
    case 2: temp = setFilterMask(CAN0Mask);
        if (temp == 0x11)
        {
            can1.setFilterMask0(CAN0Filter, CAN0Mask);
            temp = 0;
        }
        break;
    case 3: temp = setFilterMask(CAN1Filter);
        if (temp == 0x11)
        {
            can1.setFilterMask1(CAN1Filter, CAN1Mask);
            temp = 0;
        }
        break;
    case 4: temp = setFilterMask(CAN1Mask);
        if (temp == 0x11)
        {
            can1.setFilterMask1(CAN1Filter, CAN1Mask);
            temp = 0;
        }
        break;
    }
}


/*=========================================================
    Framework Functions
===========================================================*/
//Only called once at startup to draw the menu
void drawMenu()
{
    // Draw Layout
    drawSquareBtn(0, 0, 479, 319, "", themeBackground, themeBackground, themeBackground, CENTER);
    drawSquareBtn(0, 0, 130, 319, "", menuBackground, menuBackground, menuBackground, CENTER);

    // Draw Menu Buttons
    drawRoundBtn(5, 32, 125, 83, F("CANBUS"), menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
    drawRoundBtn(5, 88, 125, 140, F("VEHTOOL"), menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
    drawRoundBtn(5, 145, 125, 197, F("RZRTOOL"), menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
    drawRoundBtn(5, 202, 125, 254, F("EXTRAFN"), menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
    drawRoundBtn(5, 259, 125, 312, F("SETTING"), menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
}

//Manages the different App pages
void pageControl()
{
    // Check menu buttons for input
    menuButtons();
    switch (page)
    {
    case 0: /*========== CANBUS ==========*/
        if (!hasDrawn)
        {
            // Draw Page
            if (graphicLoaderState < 13)
            {
                drawCANBus();
                graphicLoaderState++;
            }
            else
            {
                hasDrawn = true;
            }
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
            isSerialOut = false;
        }
        // Call buttons if any
        readInCANMsg(selectedChannelOut);
        break;
    case 2:
        if (!hasDrawn)
        {
            hasDrawn = true;
            // Draw Page
            drawCANSerial();
            isSerialOut = true;
        }
        // Call buttons if any
        can1.SerialOutCAN(selectedChannelOut);
        break;
    case 3:
        if (!hasDrawn)
        {
            hasDrawn = true;
            // Draw Page

            // Initialize global var to 0
            var1 = 0;
            var2 = 0;
            var3 = 0;
            state = 0;
            counter1 = 1;
            isFinished = false;
        }
        // Call buttons if any
        sendFrame(0);
        break;
    case 4:
        if (!hasDrawn)
        {
            hasDrawn = true;
            // Draw Page

            // Initialize global var to 0
            var1 = 0;
            var2 = 0;
            var3 = 0;
            state = 0;
            counter1 = 1;
            isFinished = false;
        }
        // Call buttons if any
        sendFrame(1);
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
        if (!hasDrawn) // CTX
        {
            hasDrawn = true;
            // Draw Page

        }
        // Call buttons if any

        break;

    case 9: /*========== VEHTOOL ==========*/
        if (!hasDrawn)
        {
            // Draw Page
            if (graphicLoaderState < 11)
            {
                drawVehicleTools();
                graphicLoaderState++;
            }
            else
            {
                hasDrawn = true;
            }
        }
        // Call buttons if any
        VehicleToolButtons();
        break;
    case 10: // PIDSCAN
        if (!hasDrawn)
        {
            hasDrawn = true;
            
            can1.startCAN0(0x7E0, 0x7EF);
            drawPIDSCAN();
        }
        // Call buttons if any
        startPIDSCAN();
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
                scroll = 0;
                drawPIDStream();
                hasDrawn = true;
            }
            else
            {
                controlPage = 9;
                drawErrorMSG("Error", "Please Perform", "PIDSCAN First");
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
        if ((state == 1) && (counter1 < PIDSAMPLES) && (millis() - timer1 > 1000))
        {
            can1.PIDStream(CAN_PID_ID, arrayIn[var1], true);
            counter1++;
            drawErrorMSG("Samples", String(counter1), "Saved to SD");
            timer1 = millis();
        }
        if ((counter1 == PIDSAMPLES) && (state == 1) && (millis() - timer1 > 2000))
        {
            state = 0;
            scroll = 0;
            drawPIDStream();
        }
        break;

    case 12:
        if (!hasDrawn)
        {
            hasDrawn = true;
            // Draw Page
            //can1.startPID();
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

        }
        clearDTC();
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
        // Draw Page
        if (!hasDrawn)
        {
            if (graphicLoaderState < 11)
            {
                drawRZRTOOL();
                graphicLoaderState++;
            }
            else
            {
                hasDrawn = true;
            }
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
        // Draw Page
        if (!hasDrawn)
        {
            if (graphicLoaderState < 11)
            {
                drawExtraFN();
                graphicLoaderState++;
            }
            else
            {
                hasDrawn = true;
            }
        }
        // Call buttons if any
        extraFNButtons();
        break;
    case 28:
        if (!hasDrawn)
        {
            scroll = 0;
            var1 = 0;
            hasDrawn = true;
            // Draw Page
            drawCANLog();
            drawCANLogScroll();
        }
        // Call buttons if any
        CANLogButtons();
        break;
    case 29:
        if (!hasDrawn)
        {
            hasDrawn = true;
            // Draw Page

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
        extraFNButtons();

        break;
    case 31:
        if (!hasDrawn)
        {
            hasDrawn = true;
            // Draw Page
        }
        // Call buttons if any
        extraFNButtons();
        break;
    case 32:
        if (!hasDrawn)
        {
            hasDrawn = true;
            // Draw Page
            consoleTextJeep();
        }
        // Call buttons if any
        extraFNButtons();
        break;
    case 33:
        if (!hasDrawn)
        {
            hasDrawn = true;
            // Draw Page
            disableAutoStopJeep();
        }
        // Call buttons if any
        extraFNButtons();
        break;
    case 34:
        if (!hasDrawn)
        {
            hasDrawn = true;
            // Draw Page
        }
        // Call buttons if any
        extraFNButtons();
        break;
    case 35:
        if (!hasDrawn)
        {
            hasDrawn = true;
            // Draw Page
            timer2 = 0;
        }
        if (graphicLoaderState < 20)
        {
            drawDongleSim();
            graphicLoaderState++;
        }
        // Call buttons if any
        dongleSimButtons();
        break;

    case 36: /*========== SETTINGS ==========*/
        // Draw Page
        if (!hasDrawn)
        {
            if (graphicLoaderState < 11)
            {
                drawSettings();
                graphicLoaderState++;
            }
            else
            {
                hasDrawn = true;
            }
        }
        // Call buttons if any
        settingsButtons();
        break;
    case 37:
        if (!hasDrawn)
        {
            hasDrawn = true;
            var1 = 0;
            var4 = 0;
            state = 0;
            counter1 = 5;
            isFinished = false;
        }
        filterMask();
        break;
    case 38:
        if (!hasDrawn)
        {
            hasDrawn = true;
        }
        // Call buttons if any
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
                drawErrorMSG("Baud Rate", "Set to:", String(can1.getBaud()));
            }
            else if (temp == 250000)
            {
                can1.setBaud(500000);
                //can1.startCAN0(startRange, endRange);
                drawErrorMSG("Baud Rate", "Set to:", String(can1.getBaud()));
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
                drawErrorMSG("Baud Rate", "Set to:", String(can1.getBaud()));
            }
            else if (temp == 250000)
            {
                can1.setBaud(500000);
                //can1.startCAN0(startRange, endRange);
                drawErrorMSG("Baud Rate", "Set to:", String(can1.getBaud()));
            }
        }
        // Call buttons if any
        errorMSGButton(36);
        break;
    case 41:
        if (!hasDrawn)
        {
            hasDrawn = true;
            selectedChannelOut = 0;
            isSerialOut = false;
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

// User input keypad
void drawKeypad()
{
    drawSquareBtn(145, 55, 479, 319, "", themeBackground, themeBackground, themeBackground, CENTER);
    drawSquareBtn(180, 57, 460, 77, F("Keypad"), themeBackground, themeBackground, menuBtnColor, CENTER);
  
    uint16_t posY = 80;
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
    drawRoundBtn(365, 170, 470, 210, F("Clear"), menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
    
    drawRoundBtn(145, 270, 305, 310, F("Accept"), menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
    drawRoundBtn(315, 270, 470, 310, F("Cancel"), menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
}

// User input keypad
int keypadButtons()
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
                return 0x04;
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
                return 0x12;
            }
        }
    }
    return 0xFF;
}

// Error Message function
void drawErrorMSG(String title, String eMessage1, String eMessage2)
{
    drawSquareBtn(145, 100, 415, 220, "", menuBackground, menuBtnColor, menuBtnColor, CENTER);
    drawSquareBtn(145, 100, 415, 130, title, themeBackground, menuBtnColor, menuBtnBorder, LEFT);
    drawSquareBtn(146, 131, 414, 155, eMessage1, menuBackground, menuBackground, menuBtnText, CENTER);
    drawSquareBtn(146, 155, 414, 180, eMessage2, menuBackground, menuBackground, menuBtnText, CENTER);
    drawRoundBtn(365, 100, 415, 130, "X", menuBtnColor, menuBtnColor, menuBtnText, CENTER);
    drawRoundBtn(155, 180, 275, 215, "Confirm", menuBtnColor, menuBtnColor, menuBtnText, CENTER);
    drawRoundBtn(285, 180, 405, 215, "Cancel", menuBtnColor, menuBtnColor, menuBtnText, CENTER);
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

        if ((x >= 365) && (x <= 415))
        {
            if ((y >= 100) && (y <= 130))
            {
                waitForItRect(365, 100, 415, 130);
                return 3;
            }
        }
        if ((y >= 180) && (y <= 215))
        {
            if ((x >= 155) && (x <= 275))
            {
                waitForItRect(155, 180, 275, 215);
                return 1;
            }
            if ((x >= 285) && (x <= 405))
            {
                waitForItRect(285, 180, 405, 215);
                return 2;
            }
        }
    }
    return 0;
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
        
        if ((x >= 5) && (x <= 125))
        {
            if ((y >= 32) && (y <= 83))
            {
                // CANBUS
                waitForItMenu(5, 32, 125, 83);
                page = 0;
                hasDrawn = false;
                graphicLoaderState = 0;
            }
            if ((y >= 88) && (y <= 140))
            {
                // VEHTOOL
                waitForItMenu(5, 88, 125, 140);
                page = 9;
                hasDrawn = false;
                graphicLoaderState = 0;
            }
            if ((y >= 145) && (y <= 197))
            {
                // RZRTOOL
                waitForItMenu(5, 145, 125, 197);
                page = 18;
                hasDrawn = false;
                graphicLoaderState = 0;
            }
            if ((y >= 202) && (y <= 254))
            {
                // EXTRAFN
                waitForItMenu(5, 202, 125, 254);
                page = 27;
                hasDrawn = false;
                graphicLoaderState = 0;
            }
            if ((y >= 259) && (y <= 312))
            {
                // SETTING
                waitForItMenu(5, 259, 125, 312);
                page = 36;
                hasDrawn = false;
                graphicLoaderState = 0;
            }
        }
    }
}

// the setup function runs once when you press reset or power the board
void setup() 
{
    Serial.begin(115200);
    SerialUSB.begin(CAN_BPS_500K);

    can1.startCAN0(0x000, 0x800);
    can1.startCAN1(0x000, 0x800);

    (sdCard.startSD()) ? Serial.println("SD Running") : Serial.println("SD failed");

    // Initialize the rtc object
    rtc.begin();
#if defined UPDATE_CLOCK
    rtc.setDOW(FRIDAY);
    rtc.setTime(__TIME__);
    rtc.setDate(__DATE__);
#endif
    // LCD  and touch screen settings
    myGLCD.InitLCD();
    myGLCD.clrScr();

    myTouch.InitTouch();
    myTouch.setPrecision(PREC_MEDIUM);

    myGLCD.setFont(BigFont);
    myGLCD.setBackColor(0, 0, 255);

    drawMenu();

    // Draw the Hypertech logo
    bmpDraw("System/HYPER.bmp", 0, 0);
}

// Prints to menu
void updateTime()
{
    if (millis() - timer2 > 1000)
    {
        char time[40];
        drawRoundBtn(5, 5, 125, 30, rtc.getTimeStr(), menuBackground, menuBackground, menuBtnText, CENTER);
        timer2 = millis();
    }
}

// Sends CAN Bus traffic to serial as a background process
void serialOut()
{
    (isSerialOut) && (can1.SerialOutCAN(selectedChannelOut));
}

void waitForItBackground()
{
    if (isWaitForIt && millis() - waitForItTimer > 50)
    {
        myGLCD.setColor(menuBtnBorder);
        myGLCD.drawRoundRect(x1_, y1_, x2_, y2_);
        isWaitForIt = false;
    }
}

// Calls pageControl with a value of 1 to set view page as the home page
void loop()
{
    // GUI
    pageControl();
    
    // Background Processes
    updateTime();
    serialOut();
    waitForItBackground();
}
