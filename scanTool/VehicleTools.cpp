// 
// 
// 

#include "VehicleTools.h"
#include "common.h"
#include "definitions.h"

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
        drawRoundBtn(145, 245, 308, 295, F(""), menuBackground, menuBtnBorder, menuBtnText, CENTER);
        break;
    case 9:
        drawRoundBtn(312, 245, 475, 295, F(""), menuBackground, menuBtnBorder, menuBtnText, CENTER);
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
                waitForIt(145, 245, 308, 295);
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
                waitForIt(312, 245, 475, 295);
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
    if ((millis() - timer1 > 200) && !isFinished)
    {
        loadBar(state);

        // Get vehicle vin, will be saved in can1 object
        const uint16_t rxid = 0x7E8;
        can1.requestVIN(rxid, true);

        loadBar(state++);
    }

    // Cycle though all available banks of PIDS
    if (nextState && (millis() - timer1 >= 100))
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


