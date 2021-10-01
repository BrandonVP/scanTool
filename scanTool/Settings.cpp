// 
// 
// 

#include "Settings.h"
#include "common.h"
#include "definitions.h"

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
        drawRoundBtn(145, 80, 308, 130, F(""), menuBackground, menuBtnBorder, menuBtnText, CENTER);
        break;
    case 3:
        drawRoundBtn(312, 80, 475, 130, F(""), menuBackground, menuBtnBorder, menuBtnText, CENTER);
        break;
    case 4:
        drawRoundBtn(145, 135, 308, 185, F(""), menuBackground, menuBtnBorder, menuBtnText, CENTER);
        break;
    case 5:
        drawRoundBtn(312, 135, 475, 185, F(""), menuBackground, menuBtnBorder, menuBtnText, CENTER);
        break;
    case 6:
        drawRoundBtn(145, 190, 308, 240, F(""), menuBackground, menuBtnBorder, menuBtnText, CENTER);
        break;
    case 7:
        drawRoundBtn(312, 190, 475, 240, F(""), menuBackground, menuBtnBorder, menuBtnText, CENTER);
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
                // Unused
                //page = 37;
                //hasDrawn = false;
            }
            if ((y >= 135) && (y <= 185))
            {
                waitForIt(145, 135, 308, 185);
                // Unused
                //page = 39;
                //hasDrawn = false;
            }
            if ((y >= 190) && (y <= 240))
            {
                waitForIt(145, 190, 308, 240);
                // Unused
                //page = 41;
                //hasDrawn = false;
            }
            if ((y >= 245) && (y <= 295))
            {
                waitForIt(145, 245, 308, 295);
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
                // Unused
                //page = 38;
                //hasDrawn = false;
            }
            if ((y >= 135) && (y <= 185))
            {
                waitForIt(312, 135, 475, 185);
                // Unused
                //page = 40;
                //hasDrawn = false;
            }
            if ((y >= 190) && (y <= 240))
            {
                waitForIt(312, 190, 475, 240);
                // Unused
                //page = 42;
                //hasDrawn = false;
            }
            if ((y >= 245) && (y <= 295))
            {
                waitForIt(312, 245, 475, 295);
                //Unused
                //page = 44;
                //hasDrawn = false;
            }
        }
    }
}