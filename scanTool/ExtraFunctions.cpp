// 
// 
// 

#include "ExtraFunctions.h"
#include "common.h"
#include "definitions.h"

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
        drawRoundBtn(140, 80, 305, 130, F(""), menuBackground, menuBtnBorder, menuBtnText, CENTER);
        break;
    case 3:
        drawRoundBtn(310, 80, 475, 130, F(""), menuBackground, menuBtnBorder, menuBtnText, CENTER);
        break;
    case 4:
        drawRoundBtn(140, 135, 305, 185, F(""), menuBackground, menuBtnBorder, menuBtnText, CENTER);
        break;
    case 5:
        drawRoundBtn(310, 135, 475, 185, F(""), menuBackground, menuBtnBorder, menuBtnText, CENTER);
        break;
    case 6:
        drawRoundBtn(140, 190, 305, 240, F(""), menuBackground, menuBtnBorder, menuBtnText, CENTER);
        break;
    case 7:
        drawRoundBtn(310, 190, 475, 240, F(""), menuBackground, menuBtnBorder, menuBtnText, CENTER);
        break;
    case 8:
        drawRoundBtn(140, 245, 305, 295, F("Dongle F"), menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
        break;
    case 9:
        drawRoundBtn(310, 245, 475, 295, F("Dongle G"), menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
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
        if ((x >= 140) && (x <= 305))
        {
            if ((y >= 80) && (y <= 130))
            {
                waitForIt(140, 80, 305, 130);
                // Unused
                //page = 28;
                //hasDrawn = false;
            }
            if ((y >= 135) && (y <= 185))
            {
                waitForIt(140, 135, 305, 185);
                // Unused
                //page = 30;
                //hasDrawn = false;
            }
            if ((y >= 190) && (y <= 240))
            {
                waitForIt(140, 190, 305, 240);
                // Unused
                //page = 32;
                //hasDrawn = false;
            }
            if ((y >= 245) && (y <= 295))
            {
                waitForIt(140, 245, 305, 295);
                // Ford Dongle Simulator
                page = 34;
                hasDrawn = false;
                graphicLoaderState = 0;
            }
        }
        if ((x >= 310) && (x <= 475))
        {
            if ((y >= 80) && (y <= 130))
            {
                waitForIt(310, 80, 475, 130);
                // Unused
                //page = 29;
                //hasDrawn = false;
            }
            if ((y >= 135) && (y <= 185))
            {
                waitForIt(310, 135, 475, 185);
                // Unused
                //page = 31;
                //hasDrawn = false;
            }
            if ((y >= 190) && (y <= 240))
            {
                waitForIt(310, 190, 475, 240);
                // Unused
                //page = 33;
                //hasDrawn = false;
            }
            if ((y >= 245) && (y <= 295))
            {
                waitForIt(310, 245, 475, 295);
                // GM Dongle Simulator
                page = 35;
                hasDrawn = false;
                graphicLoaderState = 0;
            }
        }
    }
}

// Test deck for Terminator (GM)
void drawDongleSim()
{
    switch (graphicLoaderState)
    {
    case 0:
        drawSquareBtn(131, 55, 479, 319, "", themeBackground, themeBackground, themeBackground, CENTER);
        break;
    case 1:
        drawRoundBtn(145, 55, 250, 100, F("VIN"), menuBackground, menuBtnBorder, menuBtnText, CENTER);
        break;
    case 2:
        drawRoundBtn(255, 55, 325, 100, F("1"), menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
        break;
    case 3:
        drawRoundBtn(330, 55, 400, 100, F("2"), menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
        break;
    case 4:
        drawRoundBtn(405, 55, 475, 100, F("3"), menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
        break;
    case 5:
        drawRoundBtn(145, 105, 250, 150, F("0x09C"), menuBackground, menuBtnBorder, menuBtnText, CENTER);
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
        drawRoundBtn(145, 205, 250, 250, F("BCM"), menuBackground, menuBtnBorder, menuBtnText, CENTER);
        break;
    case 13:
        drawRoundBtn(255, 205, 325, 250, F("Off"), menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
        break;
    case 14:
        drawRoundBtn(330, 205, 400, 250, F("Acc"), menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
        break;
    case 15:
        drawRoundBtn(405, 205, 475, 250, F("On"), menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
        break;
    case 16:
        drawRoundBtn(145, 255, 250, 300, F("Reset"), menuBackground, menuBtnBorder, menuBtnText, CENTER);
        break;
    case 17:
        drawRoundBtn(255, 255, 475, 300, F("Send"), menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
        break;
    case 18:
        drawSquareBtn(150, 301, 479, 319, VERSION, themeBackground, themeBackground, menuBtnColor, CENTER);
        break;
    }
}

//  Test deck buttons for Terminator (GM)
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
            }
            if ((x >= 330) && (x <= 400))
            {
                waitForIt(255, 55, 325, 100);
                // 2
                const uint16_t id = 0x7E8;
                uint8_t data1[8] = { 0x21, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37 };
                can1.sendFrame(id, data1, 8, selectedChannelOut);
            }
            if ((x >= 405) && (x <= 475))
            {
                waitForIt(255, 55, 325, 100);
                // 3
                const uint16_t id = 0x7E8;
                uint8_t data1[8] = { 0x22, 0x38, 0x39, 0x41, 0x42, 0x43, 0x44, 0x45 };
                can1.sendFrame(id, data1, 8, selectedChannelOut);
            }
        }

        // Engine GM
        if ((y >= 105) && (y <= 150) && millis() - timer2 > 20)
        {
            const uint16_t id = 0x0C9;
            if ((x >= 255) && (x <= 325))
            {
                waitForIt(255, 105, 325, 150);
                // Off
                uint8_t data[8] = { 0x00, 0x02, 0x00, 0x27, 0x00, 0x00, 0x00, 0x00 };
                can1.sendFrame(id, data, 8, selectedChannelOut);
            }
            if ((x >= 330) && (x <= 400))
            {
                waitForIt(330, 105, 400, 150);
                // Off
                uint8_t data[8] = { 0x00, 0x02, 0x00, 0x27, 0x00, 0x00, 0x10, 0x00 };
                can1.sendFrame(id, data, 8, selectedChannelOut);
            }
            if ((x >= 405) && (x <= 475))
            {
                waitForIt(405, 105, 475, 150);
                // On
                uint8_t data[8] = { 0x84, 0x02, 0x00, 0x27, 0x00, 0x00, 0x30, 0x00 };
                can1.sendFrame(id, data, 8, selectedChannelOut);
            }
        }

        // ECU GM
        if ((y >= 155) && (y <= 200) && millis() - timer2 > 20)
        {
            if ((x >= 255) && (x <= 362))
            {
                waitForIt(255, 155, 362, 200);
                // Accept
                const uint16_t id = 0x7E8;
                uint8_t data[8] = { 0x02, 0xEE, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00 };
                can1.sendFrame(id, data, 8, selectedChannelOut);
            }
            if ((x >= 368) && (x <= 475))
            {
                waitForIt(368, 155, 475, 200);
                // Reject
                const uint16_t id = 0x7E8;
                uint8_t data[8] = { 0x05, 0x7F, 0xAE, 0xE3, 0x00, 0x10, 0xAA, 0xAA };
                can1.sendFrame(id, data, 8, selectedChannelOut);
            }
        }

        // BCM
        if ((y >= 205) && (y <= 250) && millis() - timer2 > 20)
        {
            const uint16_t id = 0x641;
            if ((x >= 255) && (x <= 325))
            {
                waitForIt(255, 205, 325, 250);
                uint8_t data[8] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
                can1.sendFrame(id, data, 8, selectedChannelOut);
            }
            if ((x >= 330) && (x <= 400))
            {
                waitForIt(330, 205, 400, 250);
                uint8_t data[8] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00 };
                can1.sendFrame(id, data, 8, selectedChannelOut);
            }
            if ((x >= 405) && (x <= 475))
            {
                waitForIt(405, 205, 475, 250);
                uint8_t data[8] = { 0x00, 0x00, 0x00, 0x00, 0x02, 0x02, 0x00, 0x00 };
                can1.sendFrame(id, data, 8, selectedChannelOut);
            }
        }
        // Reset Dongle
        if ((y >= 255) && (y <= 300) && millis() - timer2 > 20)
        {
            if ((x >= 255) && (x <= 475))
            {
                waitForIt(255, 255, 475, 300);
                resetDongle();
            }
        }
    }
}

// Test deck for Terminator (Ford)
void drawDongleSimFord()
{
    switch (graphicLoaderState)
    {
    case 0:
        drawSquareBtn(131, 55, 479, 319, "", themeBackground, themeBackground, themeBackground, CENTER);
        break;
    case 1:
        drawRoundBtn(145, 55, 250, 100, F("VIN"), menuBackground, menuBtnBorder, menuBtnText, CENTER);
        break;
    case 2:
        drawRoundBtn(255, 55, 325, 100, F("1"), menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
        break;
    case 3:
        drawRoundBtn(330, 55, 400, 100, F("2"), menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
        break;
    case 4:
        drawRoundBtn(405, 55, 475, 100, F("3"), menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
        break;
    case 5:
        drawRoundBtn(145, 105, 250, 150, F("Engine"), menuBackground, menuBtnBorder, menuBtnText, CENTER);
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
        drawRoundBtn(145, 155, 250, 200, F(""), menuBackground, menuBtnBorder, menuBtnText, CENTER);
        break;
    case 10:
        drawRoundBtn(255, 155, 362, 200, F(""), menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
        break;
    case 11:
        drawRoundBtn(368, 155, 475, 200, F(""), menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
        break;
    case 12:
        drawRoundBtn(145, 205, 250, 250, F(""), menuBackground, menuBtnBorder, menuBtnText, CENTER);
        break;
    case 13:
        drawRoundBtn(255, 205, 325, 250, F(""), menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
        break;
    case 14:
        drawRoundBtn(330, 205, 400, 250, F(""), menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
        break;
    case 15:
        drawRoundBtn(405, 205, 475, 250, F(""), menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
        break;
    case 16:
        drawRoundBtn(145, 255, 250, 300, F("Reset"), menuBackground, menuBtnBorder, menuBtnText, CENTER);
        break;
    case 17:
        drawRoundBtn(255, 255, 475, 300, F("Send"), menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
        break;
    case 18:
        drawSquareBtn(150, 301, 479, 319, VERSION, themeBackground, themeBackground, menuBtnColor, CENTER);
        break;
    }
}

// Test deck buttons for Terminator (Ford)
void dongleSimButtonsFord()
{
    // Touch screen controls
    if (myTouch.dataAvailable())
    {
        myTouch.read();
        x = myTouch.getX();
        y = myTouch.getY();

        // VIN G
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
        if ((y >= 105) && (y <= 150) && millis() - timer2 > 20)
        {
            const uint16_t id = 0x42F;
            if ((x >= 255) && (x <= 325))
            {
                waitForIt(255, 105, 325, 150);
                // Off
                uint8_t data[8] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
                can1.sendFrame(id, data, 8, selectedChannelOut);
            }
            if ((x >= 330) && (x <= 400))
            {
                waitForIt(330, 105, 400, 150);
                // Acc
                uint8_t data[8] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00 };
                can1.sendFrame(id, data, 8, selectedChannelOut);
            }
            if ((x >= 405) && (x <= 475))
            {
                waitForIt(405, 105, 475, 150);
                // On
                uint8_t data[8] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x00 };
                can1.sendFrame(id, data, 8, selectedChannelOut);
            }
            timer2 = millis();
        }
        //
        if ((y >= 155) && (y <= 200) && millis() - timer2 > 20)
        {
            if ((x >= 255) && (x <= 362))
            {
                waitForIt(255, 155, 362, 200);
                // Accept
                //const uint16_t id = 0x7E8;
                //uint8_t data[8] = { 0x02, 0x60, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
                //can1.sendFrame(id, data, 8, selectedChannelOut);
            }
            if ((x >= 368) && (x <= 475))
            {
                waitForIt(368, 155, 475, 200);
                // Reject
                //const uint16_t id = 0x7E8;
                //uint8_t data[8] = { 0x02, 0x7F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
                //can1.sendFrame(id, data, 8, selectedChannelOut);
            }
            timer2 = millis();
        }
        //
        if ((y >= 205) && (y <= 250) && millis() - timer2 > 20)
        {
            if ((x >= 255) && (x <= 325))
            {
                waitForIt(255, 205, 325, 250);
                // 10
                //const uint16_t id = 0x7E8;
                //uint8_t data[8] = { 0x10, 0x00, 0x00, 0x00, 0x00, 0x31, 0x46, 0x54 };
                //can1.sendFrame(id, data, 8, selectedChannelOut);
            }
            if ((x >= 330) && (x <= 400))
            {
                waitForIt(330, 205, 400, 250);
                // 20
                //const uint16_t id = 0x7E8;
                //uint8_t data[8] = { 0x21, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37 };
                //can1.sendFrame(id, data, 8, selectedChannelOut);
            }
            if ((x >= 405) && (x <= 475))
            {
                waitForIt(405, 205, 475, 250);
                // 30
                //const uint16_t id = 0x7E8;
                //uint8_t data[8] = { 0x22, 0x38, 0x39, 0x41, 0x42, 0x43, 0x44, 0x45 };
                //can1.sendFrame(id, data, 8, selectedChannelOut);
            }
        }
        // Reset Dongle
        if ((y >= 255) && (y <= 300) && millis() - timer2 > 20)
        {
            if ((x >= 255) && (x <= 475))
            {
                waitForIt(255, 255, 475, 300);
                resetDongle();
            }
            timer2 = millis();
        }
    }
}

// Reset key for Terminator
void resetDongle()
{
    const uint16_t DONGLE_RESET = 0x1FE;
    byte RESET_KEY[8] = { 0x00, 0xFE, 0x00, 0x00, 0x00, 0x00, 0xBB, 0x13 };
    can1.sendFrame(DONGLE_RESET, RESET_KEY, 8, selectedChannelOut);
}