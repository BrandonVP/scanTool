// 
// 
// 

#include "PolarisTools.h"
#include "common.h"
#include "definitions.h"

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
        drawRoundBtn(140, 80, 305, 130, F("RZR DTC"), menuBackground, menuBtnBorder, menuBtnText, CENTER);
        break;
    case 3:
        drawRoundBtn(310, 80, 475, 130, F("Pol DTC"), menuBackground, menuBtnBorder, menuBtnText, CENTER);
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
        drawRoundBtn(140, 245, 305, 295, F(""), menuBackground, menuBtnBorder, menuBtnText, CENTER);
        break;
    case 9:
        drawRoundBtn(310, 245, 475, 295, F(""), menuBackground, menuBtnBorder, menuBtnText, CENTER);
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
        if ((x >= 140) && (x <= 305))
        {
            if ((y >= 80) && (y <= 130))
            {
                waitForIt(140, 80, 305, 130);
                // Scan DTC
                //page = 19;
                //hasDrawn = false;
            }
            if ((y >= 135) && (y <= 185))
            {
                waitForIt(140, 135, 305, 185);
                // Unused
                //page = 21;
                //hasDrawn = false;
            }
            if ((y >= 190) && (y <= 240))
            {
                waitForIt(140, 190, 305, 240);
                // Unused
                //page = 23;
                //hasDrawn = false;
            }
            if ((y >= 245) && (y <= 295))
            {
                waitForIt(140, 245, 305, 295);
                // Unused
                //page = 25;
                //hasDrawn = false;
            }
        }
        if ((x >= 310) && (x <= 475))
        {
            if ((y >= 80) && (y <= 130))
            {
                waitForIt(310, 80, 475, 130);
                // Clear DTC
                //page = 20;
                //hasDrawn = false;
            }
            if ((y >= 135) && (y <= 185))
            {
                waitForIt(310, 135, 475, 185);
                // Unused
                //page = 22;
                //hasDrawn = false;
            }
            if ((y >= 190) && (y <= 240))
            {
                waitForIt(310, 190, 475, 240);
                // Unused
                //page = 24;
                //hasDrawn = false;
            }
            if ((y >= 245) && (y <= 295))
            {
                waitForIt(310, 245, 475, 295);
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
