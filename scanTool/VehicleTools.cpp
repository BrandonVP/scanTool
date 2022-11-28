/*
 ===========================================================================
 Name        : VehicleTools.cpp
 Author      : Brandon Van Pelt
 Created	 :
 Description : CAN Bus related vehicle tools
 ===========================================================================
 */

#define _VEHICLETOOLS_C
#include "VehicleTools.h"

/*=========================================================
    Vehicle Tools
===========================================================*/
// Draw Vehicle tools menu buttons
bool drawVehicleTools()
{
    switch (graphicLoaderState)
    {
    case 0:
        break;
    case 1:
        drawSquareBtn(131, 55, 479, 319, "", themeBackground, themeBackground, themeBackground, CENTER);
        break;
    case 2:
        drawRoundBtn(140, 80, 305, 130, F("PIDSCAN"), menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
        break;
    case 3:
        drawRoundBtn(310, 80, 475, 130, F("PIDSTRM"), menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
        break;
    case 4:
        drawRoundBtn(140, 135, 305, 185, F("PID Guages"), menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
        break;
    case 5:
        drawRoundBtn(310, 135, 475, 185, F("VIN"), menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
        break;
    case 6:
        drawRoundBtn(140, 190, 305, 240, F("Scan DTC"), menuBackground, menuBtnBorder, menuBtnText, CENTER);
        break;
    case 7:
        drawRoundBtn(310, 190, 475, 240, F("Clear DTC"), menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
        break;
    case 8:
        drawRoundBtn(140, 245, 305, 295, F(""), menuBackground, menuBtnBorder, menuBtnText, CENTER);
        break;
    case 9:
        drawRoundBtn(310, 245, 475, 295, F(""), menuBackground, menuBtnBorder, menuBtnText, CENTER);
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

// Buttons to start Vehicle tool programs
void VehicleToolButtons()
{
    // Touch screen controls
    if (Touch_getXY())
    {
        // Start Scan
        if ((x >= 140) && (x <= 305))
        {
            if ((y >= 80) && (y <= 130))
            {
                waitForIt(140, 80, 305, 130);
                // PIDSCAN
                nextPage = 10;
            }
            if ((y >= 135) && (y <= 185))
            {
                waitForIt(140, 135, 305, 185);
                // PID Guages
                nextPage = 12;
                
            }
            if ((y >= 190) && (y <= 240))
            {
                waitForIt(140, 190, 305, 240);
                // DTC Scan
                nextPage = 14;
            }
            if ((y >= 245) && (y <= 295))
            {
                waitForIt(140, 245, 305, 295);
                // Unused
            }
        }
        if ((x >= 310) && (x <= 475))
        {
            if ((y >= 80) && (y <= 130))
            {
                waitForIt(310, 80, 475, 130);
                // PIDSTRM
            }
            if ((y >= 135) && (y <= 185))
            {
                waitForIt(310, 135, 475, 185);
                // VIN
                can1.setFilterMask0(0x7E8, 0x7FF);
                state = 0;
                nextPage = 13;
                
            }
            if ((y >= 190) && (y <= 240))
            {
                waitForIt(310, 190, 475, 240);
                // DTC Clears
                nextPage = 15;
                
                // Initialize state machine variables to 0
                state = 0;
            }
            if ((y >= 245) && (y <= 295))
            {
                waitForIt(310, 245, 475, 295);
                // Unused
            }
        }
    }
}

/*========== PID Scan Functions ==========*/
bool drawPIDSCAN()
{
    switch (graphicLoaderState)
    {
    case 0:
        break;
    case 1:
        drawSquareBtn(131, 55, 479, 319, "", themeBackground, themeBackground, themeBackground, CENTER);
        break;
    case 3:
        drawSquareBtn(141, 90, 479, 110, F("Scan supported PIDs"), themeBackground, themeBackground, menuBtnColor, CENTER);
        break;
    case 4:
        drawSquareBtn(141, 115, 479, 135, F("to SD Card"), themeBackground, themeBackground, menuBtnColor, CENTER);
        break;
    case 5:
        return false;
        break;
    }
    graphicLoaderState++;
    return true;
}

void startPIDSCAN()
{
    switch (state)
    {
    case 0: // Start request VIN
        (g_var8[POS0] < 5) ? g_var8[POS0] = can1.requestVIN(g_var8[POS0], true) : loadBar(++state);
        break;
    case 1: // Send PID request message
        state = can1.getPIDList(state, g_var16[POS0], g_var16[POS1]);
        break;
    case 2: // Wait for response
        state = can1.getPIDList(state, g_var16[POS0], g_var16[POS1]);
        break;
    case 3: // Reponsed received and there is another PID bank to request
        g_var16[POS0] = g_var16[POS0] + 0x20;
        g_var16[POS1]++;
        // Change state back to PID request message
        state = 1;
        state = can1.getPIDList(state, g_var16[POS0], g_var16[POS1]);
        loadBar(++g_var8[POS1]);
        break;
    case 4:
        // Fill remaining loadBar
        loadBar(DONE);

        // Activate the PIDSTRM page 
        hasPID = true;

        // Move to final state
        state = 5;
        break;
    case 5:
        // Finished
        break;
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
        itoa(arrayIn[g_var8[POS0] + 1], temp, 16);
        strcat(intOut, temp);
        if (g_var8[POS0] < sizeof(arrayIn) && arrayIn[g_var8[POS0] + 1] > 0)
        {
            drawSquareBtn(150, y, 410, y + 35, intOut, menuBackground, menuBtnBorder, menuBtnText, LEFT);
        }
        else
        {
            drawSquareBtn(150, y, 410, y + 35, "", menuBackground, menuBtnBorder, menuBtnText, LEFT);
        }
        y = y + 35;
        g_var8[POS0]++;
    }
}

void drawPIDStream()
{
    drawSquareBtn(131, 55, 479, 319, "", themeBackground, themeBackground, themeBackground, CENTER);
    myGLCD.setColor(menuBtnColor);
    myGLCD.setBackColor(themeBackground);
    drawSquareBtn(420, 80, 470, 160, F("/\\"), menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
    drawSquareBtn(420, 160, 470, 240, F("\\/"), menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
    drawPIDStreamScroll();
    drawRoundBtn(150, 275, 410, 315, F("Stream PID"), menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
}

// Buttons for the PID Stream program
void PIDStreamButtons()
{
    // Touch screen controls
    if (Touch_getXY())
    {
        if ((x >= 150) && (x <= 410))
        {
            if ((y >= 60) && (y <= 95))
            {
                waitForItRect(150, 60, 410, 95);
                SerialUSB.println(1 + g_var8[POS0]);
                g_var16[POS0] = 1 + g_var8[POS0];
            }
            if ((y >= 95) && (y <= 130))
            {
                waitForItRect(150, 95, 410, 130);
                SerialUSB.println(2 + g_var8[POS0]);
                g_var16[POS0] = 2 + g_var8[POS0];
            }
            if ((y >= 130) && (y <= 165))
            {
                waitForItRect(150, 130, 410, 165);
                SerialUSB.println(3 + g_var8[POS0]);
                g_var16[POS0] = 3 + g_var8[POS0];
            }
            if ((y >= 165) && (y <= 200))
            {
                waitForItRect(150, 165, 410, 200);
                Serial.println(4 + g_var8[POS0]);
                g_var16[POS0] = 4 + g_var8[POS0];
            }
            if ((y >= 200) && (y <= 235))
            {
                waitForItRect(150, 200, 410, 235);
                SerialUSB.println(5 + g_var8[POS0]);
                g_var16[POS0] = 5 + g_var8[POS0];
            }
            if ((y >= 235) && (y <= 270))
            {
                waitForItRect(150, 235, 410, 270);
                SerialUSB.println(6 + g_var8[POS0]);
                g_var16[POS0] = 6 + g_var8[POS0];
            }
        }
        if ((x >= 420) && (x <= 470))
        {
            if ((y >= 80) && (y <= 160))
            {
                waitForItRect(420, 80, 470, 160);
                if (g_var8[POS0] > 0)
                {
                    g_var8[POS0] = g_var8[POS0] - 12;
                    drawPIDStreamScroll();
                }
            }
        }
        if ((x >= 420) && (x <= 470))
        {
            if ((y >= 160) && (y <= 240))
            {
                waitForItRect(420, 160, 470, 240);
                if (g_var8[POS0] < 100)
                {
                    drawPIDStreamScroll();
                }
            }
        }
        if ((x >= 150) && (x <= 410))
        {
            if ((y >= 275) && (y <= 315))
            {
                if (g_var16[POS0] != 0)
                {
                    waitForItRect(150, 275, 410, 315);
                    Serial.print("Sending PID: ");
                    Serial.println(arrayIn[g_var16[POS0] + 1]);
                    state = 1;
                    g_var16[POS1] = 0;
                    g_var32[POS0] = 0;
                }
            }
        }
    }
}

void streamPIDS()
{
    if (hasPID == true)
    {
        PIDStreamButtons();
    }
    else
    {
        errorMSGButton(1, 2, 3);
    }
    if ((state == 1) && (g_var16[POS1] < PIDSAMPLES) && (millis() - g_var32[POS0] > 1000))
    {
        // TODO: Fix me
        //can1.PIDStream(CAN_PID_ID, arrayIn[g_var16[POS0]], true);
        uint8_t PIDRequest[8] = { 0x02, 0x01, arrayIn[g_var16[POS0]], 0x00, 0x00, 0x00, 0x00, 0x00 };
        can1.sendFrame(CAN_PID_ID, PIDRequest, 8, false);
        g_var16[POS1]++;
        drawErrorMSG2("Samples", String(g_var16[POS1]), "Saved to SD");
        g_var32[POS0] = millis();
        uint8_t result = 0;
        uint32_t wait = millis();
        while (millis() - wait < 10)
        {
            backgroundProcess();
        }
        can1.PIDStream(result, true);
    }
    if ((g_var16[POS1] == PIDSAMPLES) && (state == 1) && (millis() - g_var32[POS0] > 2000))
    {
        state = 0;
        g_var8[POS0] = 0;
        drawPIDStream();
        g_var32[POS0] = millis(); //
    }
}

/*============== PID Stream Guages ==============*/
// Draw the gauges
bool drawPIDGauges()
{
    drawSquareBtn(131, 60, 479, 319, "", themeBackground, themeBackground, themeBackground, CENTER);

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
    myGLCD.print(F("Load"), 188, 148); // 0x04
    myGLCD.print(F("RPM"), 358, 148);  // 0x0C
    myGLCD.print(F("TEMP"), 188, 278); // 0x05
    myGLCD.print(F("MPH"), 358, 278);  // 0x0D
    myGLCD.setBackColor(menuBtnColor);
}

// 
void PIDGauges()
{
    const float pi = 3.14159;
    const uint8_t r = 50;
    const float offset = (pi / 2) + 1;

    uint16_t x1 = r * cos(pi / 2) + 220;
    uint16_t y1 = r * sin(pi / 2) + 120;
    uint16_t x2 = r * cos(pi / 2) + 380;
    uint16_t y2 = r * sin(pi / 2) + 120;
    uint16_t x3 = r * cos(pi / 2) + 220;
    uint16_t y3 = r * sin(pi / 2) + 250;
    uint16_t x4 = r * cos(pi / 2) + 380;
    uint16_t y4 = r * sin(pi / 2) + 250;

    bool isWait = true;
    float g1, g2, g3, g4 = -1;

    uint8_t PIDRequest[8] = { 0x02, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

    uint32_t sendTimer = 0;
    uint8_t traverse = 0;
    uint8_t value = 0;

    while (isWait)
    {
        if (Touch_getXY())
        {
            if ((x >= 1) && (x <= 140))
            {
                if ((y >= 1) && (y <= 319))
                {
                    isWait = false;
                }
            }
        }

        if (millis() - sendTimer > 3)
        {
            uint8_t temp = traverse & 0x7;
            switch (temp)
            {
            case 0:
                PIDRequest[2] = 0x4;
                can1.sendFrame(CAN_PID_ID, PIDRequest, 8, false);
                break;
            case 1:
                break;
            case 2:
                PIDRequest[2] = 0xC;
                can1.sendFrame(CAN_PID_ID, PIDRequest, 8, false);
                break;
            case 3:
                break;
            case 4:
                PIDRequest[2] = 0x5;
                can1.sendFrame(CAN_PID_ID, PIDRequest, 8, false);
                break;
            case 5:
                break;
            case 6:
                PIDRequest[2] = 0xD;
                can1.sendFrame(CAN_PID_ID, PIDRequest, 8, false);
                break;
            case 7:
                break;
            }
            sendTimer = millis();
            traverse++;
        }

        uint8_t result = can1.PIDStream(value, false);

        myGLCD.setBackColor(VGA_WHITE);
        // gauge values 0-286

        if (result == 0x04)
        {
            g1 = value;
            myGLCD.printNumI(g1, 197, 128, 3, '0');
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
        if (result == 0x0C)
        {
            g2 = value;
            myGLCD.printNumI(g2, 343, 128, 5, '0');
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
        if (result == 0x05)
        {
            g3 = value;
            myGLCD.printNumI(g3, 193, 258, 3, '0');
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
        if (result == 0x0D)
        {
            g4 = value;
            myGLCD.printNumI(g4, 358, 258, 3, '0');
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
}

/*=============== Vin ================*/
void drawVIN()
{
    const uint16_t rxid = 0x7E8;
    drawSquareBtn(131, 55, 479, 319, "", themeBackground, themeBackground, themeBackground, CENTER);
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
    if (millis() - g_var32[POS0] > 500 && !isFinished)
    {
        drawSquareBtn(131, 60, 479, 319, "", themeBackground, themeBackground, themeBackground, CENTER);
        drawSquareBtn(150, 150, 479, 170, F("Clearing DTCS..."), themeBackground, themeBackground, menuBtnColor, CENTER);
    }

    // Cycle through clear messages
    if (state < 7)
    {
        if (millis() - g_var32[POS0] >= 400)
        {
            uint32_t IDc[7] = { 0x7D0, 0x720, 0x765, 0x737, 0x736, 0x721, 0x760 };
            byte MSGc[8] = { 0x4, 0x18, 0x00, 0xFF, 0x00, 0x55, 0x55, 0x55 };

            can1.sendFrame(IDc[state], MSGc, 8, selectedChannelOut);
            g_var16[POS1]++;
            g_var32[POS0] = millis();
        }
        if (g_var16[POS1] == 3)
        {
            g_var16[POS1] = 0;
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

void drawClearDTC()
{
    drawSquareBtn(131, 60, 479, 319, "", themeBackground, themeBackground, themeBackground, CENTER);
    drawSquareBtn(150, 135, 460, 155, F("Turn key to run"), themeBackground, themeBackground, menuBtnColor, CENTER);
    drawSquareBtn(150, 160, 460, 180, F("with engine off"), themeBackground, themeBackground, menuBtnColor, CENTER);
    drawRoundBtn(140, 190, 305, 240, F("Scan DTC"), menuBackground, menuBtnBorder, menuBtnText, CENTER);
    drawRoundBtn(310, 190, 475, 240, F("Clear DTC"), menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
}

// Buttons for the DTC program
uint8_t DTCButtons()
{
    // Touch screen controls
    if (Touch_getXY())
    {
        // Start Scan
        if ((y >= 190) && (y <= 240))
        {
            if ((x >= 140) && (x <= 305))
            {
                waitForIt(140, 190, 305, 240);
                // Scan DTCs
                
            }
            if ((x >= 310) && (x <= 475))
            {
                waitForIt(310, 190, 475, 240);
                // Clear DTCs
                return 2;
            }
        }
    }
    return 1;
}
