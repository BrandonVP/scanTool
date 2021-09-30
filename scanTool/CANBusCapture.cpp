/*
 Name:    CANBusCapture.cpp
 Created: 11/15/2020 8:54:18 AM
 Author:  Brandon Van Pelt
*/

#pragma once

#include "CANBusCapture.h"
#include "common.h"
#include "definitions.h"


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


/*
void drawCANBus()
{
    switch (graphicLoaderState)
    {
    case 0:
        break;
    case 1:
        drawRoundBtn(131, 55, 479, 319, "", themeBackground, themeBackground, themeBackground, CENTER);
        break;
    case 2:
        drawRoundBtn(230, 155, 330, 200, F("Serial"), menuBtnColor, menuBtnBorder, menuBtnText, CENTER); //1
        
        break;
    case 3:
        
        break;
    case 4:
        drawRoundBtn(335, 155, 400, 200, F("LCD"), menuBtnColor, menuBtnBorder, menuBtnText, CENTER); //2
        break;
    case 5:
      
        break;
    case 6:
        drawRoundBtn(230, 105, 330, 150, F("Serial"), menuBtnColor, menuBtnBorder, menuBtnText, CENTER); //3
        break;
    case 7:
      
        
        break;
    case 8:
        drawRoundBtn(335, 205, 400, 250, F("LCD"), menuBtnColor, menuBtnBorder, menuBtnText, CENTER); //4
        
        break;
    case 9:
        
        break;
    case 10:
        drawRoundBtn(230, 205, 330, 250, F("Serial"), menuBtnColor, menuBtnBorder, menuBtnText, CENTER); //5
        break;
    case 11:
        
        break;
    case 12:
        drawRoundBtn(335, 105, 400, 150, F("LCD"), menuBtnColor, menuBtnBorder, menuBtnText, CENTER); //6
        break;
    case 13:
        
        break;
    case 14:
        drawRoundBtn(135, 155, 225, 200, F("Both"), menuBackground, menuBtnBorder, menuBtnText, CENTER); //7
        break;
    case 15:
        
        break;
    case 16:
        drawRoundBtn(405, 155, 475, 200, F("SD"), menuBtnColor, menuBtnBorder, menuBtnText, CENTER); //8
        break;
    case 17:
        
        break;
    case 18:
        drawRoundBtn(335, 255, 400, 300, F("LCD"), menuBtnColor, menuBtnBorder, menuBtnText, CENTER); //9
        break;
    case 19:
        
        break;
    case 20:
        drawRoundBtn(405, 205, 475, 250, F("SD"), menuBtnColor, menuBtnBorder, menuBtnText, CENTER); //10
        break;
    case 21:
        
        break;
    case 22:
        drawRoundBtn(135, 205, 225, 250, F("C1TX"), menuBackground, menuBtnBorder, menuBtnText, CENTER); //11
        break;
    case 23:
        
        break;
    case 24:
        drawRoundBtn(405, 105, 475, 150, F("SD"), menuBtnColor, menuBtnBorder, menuBtnText, CENTER); //12
        break;
        
        break;
    case 26:
        drawRoundBtn(135, 105, 225, 150, F("CAN1"), menuBackground, menuBtnBorder, menuBtnText, CENTER); //13
        break;
    case 27:
       
        break;
    case 28:
        drawRoundBtn(230, 55, 330, 100, F("Serial"), menuBtnColor, menuBtnBorder, menuBtnText, CENTER); //14
        break;
    case 29:

        break;
    case 30:
        drawRoundBtn(230, 255, 330, 300, F("Serial"), menuBtnColor, menuBtnBorder, menuBtnText, CENTER); //15
        break;
    case 31:
        
        break;
    case 32:
        drawRoundBtn(335, 55, 405, 100, F("LCD"), menuBtnColor, menuBtnBorder, menuBtnText, CENTER); //16
        break;
    case 33:
 
        break;
    case 34:
        drawRoundBtn(405, 255, 475, 300, F("SD"), menuBtnColor, menuBtnBorder, menuBtnText, CENTER); //17
        break;
    case 35:

        break;
    case 36:
        drawRoundBtn(135, 55, 225, 100, F("CAN0"), menuBackground, menuBtnBorder, menuBtnText, CENTER); //18
        break;
    case 37:

        break;
    case 38:
        drawRoundBtn(410, 55, 475, 100, F("SD"), menuBtnColor, menuBtnBorder, menuBtnText, CENTER); //19
        break;
    case 39:
        
        break;
    case 40:
        drawRoundBtn(135, 255, 225, 300, F("C1C2"), menuBackground, menuBtnBorder, menuBtnText, CENTER);//20
        break;
    case 41:
        drawSquareBtn(150, 301, 479, 319, VERSION, themeBackground, themeBackground, menuBtnColor, CENTER);
        break;
    }
}
*/
/*
void drawCANBus()
{
    switch (graphicLoaderState)
    {
    case 0:
        break;
    case 1:
        drawRoundBtn(131, 55, 479, 319, "", themeBackground, themeBackground, themeBackground, CENTER);
        break;
    case 2:
        drawRoundBtn(135, 55, 225, 100, F("CAN0"), menuBackground, menuBtnBorder, menuBtnText, CENTER);
        break;
    case 3:
        drawRoundBtn(230, 55, 330, 100, F("Serial"), menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
        break;
    case 4:
        drawRoundBtn(335, 55, 405, 100, F("LCD"), menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
        break;
    case 5:
        drawRoundBtn(410, 55, 475, 100, F("SD"), menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
        break;
    case 6:
        drawRoundBtn(135, 105, 225, 150, F("CAN1"), menuBackground, menuBtnBorder, menuBtnText, CENTER);
        break;
    case 7:
        drawRoundBtn(230, 105, 330, 150, F("Serial"), menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
        break;
    case 8:
        drawRoundBtn(335, 105, 400, 150, F("LCD"), menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
        break;
    case 9:
        drawRoundBtn(405, 105, 475, 150, F("SD"), menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
        break;
    case 10:
        drawRoundBtn(135, 155, 225, 200, F("Both"), menuBackground, menuBtnBorder, menuBtnText, CENTER);
        break;
    case 11:
        drawRoundBtn(230, 155, 330, 200, F("Serial"), menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
        break;
    case 12:
        drawRoundBtn(335, 155, 400, 200, F("LCD"), menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
        break;
    case 13:
        drawRoundBtn(405, 155, 475, 200, F("SD"), menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
        break;
    case 14:
        drawRoundBtn(135, 205, 225, 250, F("C1TX"), menuBackground, menuBtnBorder, menuBtnText, CENTER);
        break;
    case 15:
        drawRoundBtn(230, 205, 330, 250, F("Serial"), menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
        break;
    case 16:
        drawRoundBtn(335, 205, 400, 250, F("LCD"), menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
        break;
    case 17:
        drawRoundBtn(405, 205, 475, 250, F("SD"), menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
        break;
    case 18:
        drawRoundBtn(135, 255, 225, 300, F("C1C2"), menuBackground, menuBtnBorder, menuBtnText, CENTER);
        break;
    case 19:
        drawRoundBtn(230, 255, 330, 300, F("Serial"), menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
        break;
    case 20:
        drawRoundBtn(335, 255, 400, 300, F("LCD"), menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
        break;
    case 21:
        drawRoundBtn(405, 255, 475, 300, F("SD"), menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
        break;
    case 22:
        drawSquareBtn(150, 301, 479, 319, VERSION, themeBackground, themeBackground, menuBtnColor, CENTER);
        break;
    }
}
*/
/*
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
        drawSquareBtn(131, 55, 245, 85, F("Source"), menuBackground, menuBtnBorder, menuBtnText, CENTER);
        drawSquareBtn(131, 85, 245, 125, F("CAN0 RX"), menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
        drawSquareBtn(131, 125, 245, 165, F("CAN1 RX"), menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
        drawSquareBtn(131, 165, 245, 205, F("CAN0 TX"), menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
        drawSquareBtn(131, 205, 245, 245, F("CAN1 TX"), menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
        break;
    case 3:
        drawSquareBtn(245, 55, 360, 85, F("Capture"), menuBackground, menuBtnBorder, menuBtnText, CENTER);
        drawSquareBtn(245, 85, 360, 125, F("LCD"), menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
        drawSquareBtn(245, 125, 360, 165, F("Serial"), menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
        drawSquareBtn(245, 165, 360, 205, F("Serial"), menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
        break;
    case 4:
        drawSquareBtn(360, 55, 475, 85, F("Output"), menuBackground, menuBtnBorder, menuBtnText, CENTER);
        drawSquareBtn(360, 85, 475, 125, F("LCD"), menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
        drawSquareBtn(360, 125, 475, 165, F("Serial"), menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
        drawSquareBtn(360, 165, 475, 205, F("SD Card"), menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
        break;
    case 5:
        
        break;
    case 6:
        
        break;
    case 7:
        
        break;
    case 8:
        //drawSquareBtn(145, 205, 308, 250, F("CTX0: LCD"), menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
        break;
    case 9:
        //drawSquareBtn(312, 205, 475, 250, F("Serial"), menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
        break;
    case 10:
        //drawSquareBtn(145, 255, 308, 300, F("CAN0 RX"), menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
        break;
    case 11:
        //drawSquareBtn(312, 255, 475, 300, F("CAN1 RX"), menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
        break;
    case 12:
        drawSquareBtn(150, 301, 479, 319, VERSION, themeBackground, themeBackground, menuBtnColor, CENTER);
        break;
    }
}
*/
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
    drawSquareBtn(131, 55, 479, 319, "", themeBackground, themeBackground, themeBackground, CENTER);
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

//
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
    // Clear
    if (var2 == 0x10)
    {
        counter1 = 1;
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
