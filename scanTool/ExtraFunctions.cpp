// 
// 
// 

#include "ExtraFunctions.h"
#include "common.h"
#include "definitions.h"

/*=========================================================
    Extra Functions
===========================================================*/
// Draw buttons to start extra function programs
bool drawExtraFN()
{
    switch (graphicLoaderState)
    {
    case 0:
        break;
    case 1:
        drawSquareBtn(131, 55, 479, 319, "", themeBackground, themeBackground, themeBackground, CENTER);
        break;
    case 2:
        drawRoundBtn(140, 80, 305, 130, F("OBD Sim"), menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
        break;
    case 3:
        drawRoundBtn(310, 80, 475, 130, F("MSG Spam"), menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
        break;
    case 4:
        drawRoundBtn(140, 135, 305, 185, F("Keyboard"), menuBackground, menuBtnBorder, menuBtnText, CENTER);
        break;
    case 5:
        drawRoundBtn(310, 135, 475, 185, F("Keypad H"), menuBackground, menuBtnBorder, menuBtnText, CENTER);
        break;
    case 6:
        drawRoundBtn(140, 190, 305, 240, F("Keypad D"), menuBackground, menuBtnBorder, menuBtnText, CENTER);
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
    case 11: 
        return false;
        break;
    }
    graphicLoaderState++;
    return true;
}

// Buttons to start extra function programs
void extraFNButtons()
{
    // Touch screen controls
    if (Touch_getXY())
    {
        if ((x >= 140) && (x <= 305))
        {
            if ((y >= 80) && (y <= 130))
            {
                waitForIt(140, 80, 305, 130);
                // OBD Simulator
                nextPage = 28;
            }
            if ((y >= 135) && (y <= 185))
            {
                waitForIt(140, 135, 305, 185);
                // Keyboard
                nextPage = 30;
                hasDrawn = false;
            }
            if ((y >= 190) && (y <= 240))
            {
                waitForIt(140, 190, 305, 240);
                // Keypad D
                nextPage = 32;
            }
            if ((y >= 245) && (y <= 295))
            {
                waitForIt(140, 245, 305, 295);
                // Ford Dongle Simulator
                nextPage = 34;
                graphicLoaderState = 0;
            }
        }
        if ((x >= 310) && (x <= 475))
        {
            if ((y >= 80) && (y <= 130))
            {
                waitForIt(310, 80, 475, 130);
                // Message Spam
                graphicLoaderState = 0;
                state = 0;
                nextPage = 29;
            }
            if ((y >= 135) && (y <= 185))
            {
                waitForIt(310, 135, 475, 185);
                // Keypad H
                nextPage = 31;
            }
            if ((y >= 190) && (y <= 240))
            {
                waitForIt(310, 190, 475, 240);
                // Unused
                //nextPage = 33;
            }
            if ((y >= 245) && (y <= 295))
            {
                waitForIt(310, 245, 475, 295);
                // GM Dongle Simulator
                nextPage = 35;
                
                graphicLoaderState = 0;
            }
        }
    }
}

/*============== OBD Simulator ==============*/

// Draw the OBD Simulator page
void drawOBDSimulator()
{
    drawSquareBtn(131, 55, 479, 319, "", themeBackground, themeBackground, themeBackground, CENTER);
    drawSquareBtn(131, 120, 479, 140, F("OBD Simulator Started"), themeBackground, themeBackground, menuBtnColor, CENTER);
 }

/*============== Message Spam ==============*/

// Draw the Message Spam page
void drawMSGSpam()
{
    drawSquareBtn(131, 55, 479, 319, "", themeBackground, themeBackground, themeBackground, CENTER);
    drawRoundBtn(135, 180, 240, 240, F("Min:"), menuBackground, menuBtnBorder, menuBtnText, CENTER);
    drawRoundBtn(135, 115, 240, 175, F("Max:"), menuBackground, menuBtnBorder, menuBtnText, CENTER);
    drawSquareBtn(290, 90, 479, 110, F("Interval(ms)"), themeBackground, themeBackground, menuBtnColor, CENTER);
    drawRoundBtn(415, 115, 475, 175, F("/\\"), menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
    drawRoundBtn(415, 180, 475, 240, F("\\/"), menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
    drawRoundBtn(135, 255, 305, 305, F("Start"), menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
    drawRoundBtn(310, 255, 475, 305, F("Stop"), menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
}

void drawMSGSpamMin()
{
    drawRoundBtn(245, 180, 320, 240, String(g_var16[POS2], 16), menuBtnColor, menuBtnBorder, menuBtnText, CENTER); 
}

void drawMSGSpamMax()
{
    drawRoundBtn(245, 115, 320, 175, String(g_var16[POS1], 16), menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
}

void drawMSGInterval()
{
    drawRoundBtn(325, 150, 410, 205, String(g_var16[POS3]), menuBackground, menuBtnBorder, menuBtnText, CENTER);
}

void incID()
{
    // Current ID is greater than max ID
    if (g_var16[POS0] < g_var16[POS1])
    {
        // Increment ID
        g_var16[POS0] = g_var16[POS0] + 0x001;
    }
    else
    {
        // Else set id to min value
        g_var16[POS0] = g_var16[POS2];
    }
}

void sendMSGButtons()
{
    if (Touch_getXY())
    {
        if ((y >= 255) && (y <= 305))
        {
            if ((x >= 135) && (x <= 305))
            {
                waitForIt(135, 255, 305, 305);
                // Start
                g_var16[POS0] = g_var16[POS2];
                g_var8[POS0] = true;
            }
            if ((x >= 310) && (x <= 475))
            {
                waitForIt(310, 255, 475, 305);
                // Stop
                g_var8[POS0] = false;
            }
        }
        if ((x >= 325) && (x <= 410))
        {
            if ((y >= 150) && (y <= 205))
            {
                waitForIt(325, 150, 410, 205);
                // Interval
                state = 1;
            }
        }
        if ((x >= 245) && (x <= 320))
        {
            if ((y >= 180) && (y <= 240))
            {
                waitForIt(245, 180, 320, 240);
                // Min
                state = 2;
            }
            if ((y >= 115) && (y <= 175))
            {
                waitForIt(245, 115, 320, 175);
                // Max
                state = 3;
            }
        }
        if ((x >= 415) && (x <= 475))
        {
            if ((y >= 115) && (y <= 175))
            {
                waitForIt(415, 115, 475, 175);
                // Up
                g_var16[POS3]++;
                drawMSGInterval();
            }
            if ((y >= 180) && (y <= 240))
            {
                waitForIt(415, 180, 475, 240);
                // Down
                if (g_var16[POS3] > 0)
                {
                    g_var16[POS3]--;
                    drawMSGInterval();
                }
            }
        }
    }
}

void sendMSG()
{
    uint8_t dataOut1[8] = { 0x5F, 0x48, 0x65, 0x6C, 0x6C, 0x6F, 0x21, 0x5F };
    uint8_t dataOut2[8] = { 0x42, 0x57, 0x6F, 0x72, 0x6C, 0x64, 0x00, 0x00 };
    uint8_t dataOut3[8] = { 0x42, 0x2D, 0x2D, 0x2D, 0x2D, 0x2D, 0x2D, 0x2D };
    uint8_t dataOut4[8] = { 0x42, 0x34, 0x56, 0x78, 0x9A, 0xBC, 0xDE, 0xFF };
    uint8_t dataOut5[8] = { 0x42, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88 };
    uint8_t dataOut6[8] = { 0x42, 0xBC, 0xEF, 0xDA, 0xCE, 0xAF, 0xEB, 0xCD };
    uint8_t dataOut7[8] = { 0x42, 0x22, 0x22, 0x22, 0x22, 0x22, 0x22, 0x22 };
    uint8_t dataOut8[8] = { 0x42, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11 };

    can1.sendFrame(g_var16[POS0], dataOut1, 8, false);
}

void MSGSpam()
{
    if (g_var8[POS0] == 1 && millis() - g_var32[POS0] > g_var16[POS3])
    {
        sendMSG();
        incID();
        g_var32[POS0] = millis();
    }
}
/*============== Dongle Simulator ==============*/

// Draw test deck for Terminator (GM)
bool drawDongleSim()
{
    switch (graphicLoaderState)
    {
    case 0:
        break;
    case 1:
        drawSquareBtn(131, 55, 479, 319, "", themeBackground, themeBackground, themeBackground, CENTER);
        break;
    case 2:
        drawRoundBtn(145, 55, 250, 100, F("VIN"), menuBackground, menuBtnBorder, menuBtnText, CENTER);
        break;
    case 3:
        drawRoundBtn(255, 55, 325, 100, F("1"), menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
        break;
    case 4:
        drawRoundBtn(330, 55, 400, 100, F("2"), menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
        break;
    case 5:
        drawRoundBtn(405, 55, 475, 100, F("3"), menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
        break;
    case 6:
        drawRoundBtn(145, 105, 250, 150, F("0x09C"), menuBackground, menuBtnBorder, menuBtnText, CENTER);
        break;
    case 7:
        drawRoundBtn(255, 105, 325, 150, F("Off"), menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
        break;
    case 8:
        drawRoundBtn(330, 105, 400, 150, F("Acc"), menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
        break;
    case 9:
        drawRoundBtn(405, 105, 475, 150, F("On"), menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
        break;
    case 10:
        drawRoundBtn(145, 155, 250, 200, F("ECU"), menuBackground, menuBtnBorder, menuBtnText, CENTER);
        break;
    case 11:
        drawRoundBtn(255, 155, 362, 200, F("Accept"), menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
        break;
    case 12:
        drawRoundBtn(368, 155, 475, 200, F("Reject"), menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
        break;
    case 13:
        drawRoundBtn(145, 205, 250, 250, F("BCM"), menuBackground, menuBtnBorder, menuBtnText, CENTER);
        break;
    case 14:
        drawRoundBtn(255, 205, 325, 250, F("Off"), menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
        break;
    case 15:
        drawRoundBtn(330, 205, 400, 250, F("Acc"), menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
        break;
    case 16:
        drawRoundBtn(405, 205, 475, 250, F("On"), menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
        break;
    case 17:
        drawRoundBtn(145, 255, 250, 300, F("Reset"), menuBackground, menuBtnBorder, menuBtnText, CENTER);
        break;
    case 18:
        drawRoundBtn(255, 255, 475, 300, F("Send"), menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
        break;
    case 19:
        drawSquareBtn(150, 301, 479, 319, VERSION, themeBackground, themeBackground, menuBtnColor, CENTER);
        break;
    case 20:
        return false;
        break;
    }
    graphicLoaderState++;
    return true;
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
        if ((y >= 55) && (y <= 100) && millis() - g_var32[POS0] > 20)
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
                waitForIt(330, 55, 400, 100);
                // 2
                const uint16_t id = 0x7E8;
                uint8_t data1[8] = { 0x21, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37 };
                can1.sendFrame(id, data1, 8, selectedChannelOut);
            }
            if ((x >= 405) && (x <= 475))
            {
                waitForIt(405, 55, 475, 100);
                // 3
                const uint16_t id = 0x7E8;
                uint8_t data1[8] = { 0x22, 0x38, 0x39, 0x41, 0x42, 0x43, 0x44, 0x45 };
                can1.sendFrame(id, data1, 8, selectedChannelOut);
            }
        }

        // Engine GM
        if ((y >= 105) && (y <= 150) && millis() - g_var32[POS0] > 20)
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
        if ((y >= 155) && (y <= 200) && millis() - g_var32[POS0] > 20)
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
        if ((y >= 205) && (y <= 250) && millis() - g_var32[POS0] > 20)
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
        if ((y >= 255) && (y <= 300) && millis() - g_var32[POS0] > 20)
        {
            if ((x >= 255) && (x <= 475))
            {
                waitForIt(255, 255, 475, 300);
                resetDongle();
            }
        }
    }
}

// Draw test deck for Terminator (Ford)
bool drawDongleSimFord()
{
    switch (graphicLoaderState)
    {
    case 0:
        break;
    case 1:
        drawSquareBtn(131, 55, 479, 319, "", themeBackground, themeBackground, themeBackground, CENTER);
        break;
    case 2:
        drawRoundBtn(145, 55, 250, 100, F("VIN"), menuBackground, menuBtnBorder, menuBtnText, CENTER);
        break;
    case 3:
        drawRoundBtn(255, 55, 325, 100, F("1"), menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
        break;
    case 4:
        drawRoundBtn(330, 55, 400, 100, F("2"), menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
        break;
    case 5:
        drawRoundBtn(405, 55, 475, 100, F("3"), menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
        break;
    case 6:
        drawRoundBtn(145, 105, 250, 150, F("Engine"), menuBackground, menuBtnBorder, menuBtnText, CENTER);
        break;
    case 7:
        drawRoundBtn(255, 105, 325, 150, F("Off"), menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
        break;
    case 8:
        drawRoundBtn(330, 105, 400, 150, F("Acc"), menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
        break;
    case 9:
        drawRoundBtn(405, 105, 475, 150, F("On"), menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
        break;
    case 10:
        drawRoundBtn(145, 155, 250, 200, F(""), menuBackground, menuBtnBorder, menuBtnText, CENTER);
        break;
    case 11:
        drawRoundBtn(255, 155, 362, 200, F(""), menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
        break;
    case 12:
        drawRoundBtn(368, 155, 475, 200, F(""), menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
        break;
    case 13:
        drawRoundBtn(145, 205, 250, 250, F(""), menuBackground, menuBtnBorder, menuBtnText, CENTER);
        break;
    case 14:
        drawRoundBtn(255, 205, 325, 250, F("81"), menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
        break;
    case 15:
        drawRoundBtn(330, 205, 400, 250, F("01"), menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
        break;
    case 16:
        drawRoundBtn(405, 205, 475, 250, F(""), menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
        break;
    case 17:
        drawRoundBtn(145, 255, 250, 300, F("Reset"), menuBackground, menuBtnBorder, menuBtnText, CENTER);
        break;
    case 18:
        drawRoundBtn(255, 255, 475, 300, F("Send"), menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
        break;
    case 19:
        drawSquareBtn(150, 301, 479, 319, VERSION, themeBackground, themeBackground, menuBtnColor, CENTER);
        break;
    case 20:
        return false;
        break;
    }
    graphicLoaderState++;
    return true;
}

// Test deck buttons for Terminator (Ford)
void dongleSimButtonsFord()
{
    // Touch screen controls
    if (Touch_getXY())
    {
        // VIN G
        if ((y >= 55) && (y <= 100) && millis() - g_var32[POS0] > 20)
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
            g_var32[POS0] = millis();
        }
        // Engine
        if ((y >= 105) && (y <= 150) && millis() - g_var32[POS0] > 20)
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
            g_var32[POS0] = millis();
        }
        //
        if ((y >= 155) && (y <= 200) && millis() - g_var32[POS0] > 20)
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
            g_var32[POS0] = millis();
        }
        //
        if ((y >= 205) && (y <= 250) && millis() - g_var32[POS0] > 20)
        {
            const uint16_t id = 0x326;
            if ((x >= 255) && (x <= 325))
            {
                waitForIt(255, 205, 325, 250);
                // 
                uint8_t data[8] = { 0x81, 0xA0, 0x42, 0x75, 0x81, 0xA0, 0x00, 0x00 };
                can1.sendFrame(id, data, 8, selectedChannelOut);
            }
            if ((x >= 330) && (x <= 400))
            {
                waitForIt(330, 205, 400, 250);
                // 
                uint8_t data[8] = { 0x01, 0xA0, 0x42, 0x75, 0x81, 0xA0, 0x00, 0x00 };
                can1.sendFrame(id, data, 8, selectedChannelOut);
            }
            if ((x >= 405) && (x <= 475))
            {
                waitForIt(405, 205, 475, 250);
                // 
                //uint8_t data[8] = { 0x22, 0x38, 0x39, 0x41, 0x42, 0x43, 0x44, 0x45 };
                //can1.sendFrame(id, data, 8, selectedChannelOut);
            }
        }
        // Reset Dongle
        if ((y >= 255) && (y <= 300) && millis() - g_var32[POS0] > 20)
        {
            if ((x >= 255) && (x <= 475))
            {
                waitForIt(255, 255, 475, 300);
                resetDongle();
            }
            g_var32[POS0] = millis();
        }
    }
}

// Reset key for Terminator
void resetDongle()
{
    const uint16_t DONGLE_RESET = 0x1FE;
    byte RESET_KEY[8] = { 0x00, 0xFE, 0x01, 0x02, 0x03, 0x04, 0xBB, 0x13 };
    can1.sendFrame(DONGLE_RESET, RESET_KEY, 8, selectedChannelOut);
}