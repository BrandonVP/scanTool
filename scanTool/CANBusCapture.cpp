/*
 Name:    CANBusCapture.cpp
 Created: 11/15/2020 8:54:18 AM
 Author:  Brandon Van Pelt
*/

#pragma once

#include "CANBusCapture.h"
#include "common.h"
#include "definitions.h"


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
        drawSquareBtn(131, 55, 479, 319, "", themeBackground, themeBackground, themeBackground, CENTER);
        break;
    case 2:
        drawSquareBtn(133, 55, 248, 100, F("Source"), menuBackground, menuBtnBorder, menuBtnText, CENTER);
        drawSquareBtn(140, 105, 250, 150, F("CAN0"), menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
        drawSquareBtn(140, 105, 250, 150, F("CAN1"), menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
        break;
    case 3:
        drawSquareBtn(248, 55, 363, 100, F("Output"), menuBackground, menuBtnBorder, menuBtnText, CENTER);
        drawSquareBtn(250, 105, 360, 150, F("LCD"), menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
        drawSquareBtn(250, 155, 360, 200, F("Serial"), menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
        break;
    case 4:
        drawSquareBtn(363, 55, 478, 100, F("Capture"), menuBackground, menuBtnBorder, menuBtnText, CENTER);
        
        break;
    case 5:
        
        break;
    case 6:
        drawSquareBtn(145, 155, 308, 200, F("CAN1"), menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
        break;
    case 7:
        
        break;
    case 8:
        drawSquareBtn(145, 205, 308, 250, F("CTX0: LCD"), menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
        break;
    case 9:
        drawSquareBtn(312, 205, 475, 250, F("Serial"), menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
        break;
    case 10:
        drawSquareBtn(145, 255, 308, 300, F("CAN0 RX"), menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
        break;
    case 11:
        drawSquareBtn(312, 255, 475, 300, F("CAN1 RX"), menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
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