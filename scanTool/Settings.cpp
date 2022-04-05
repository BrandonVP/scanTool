// 
// 
// 

#include "Settings.h"
#include "common.h"
#include "definitions.h"

/*=========================================================
    Settings
===========================================================*/
// Draw setting page
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
        drawRoundBtn(140, 80, 305, 130, F("Memory"), menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
        break;
    case 3:
        drawRoundBtn(310, 80, 475, 130, F("About"), menuBackground, menuBtnBorder, menuBtnText, CENTER);
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

// Buttons to start setting programs
void settingsButtons()
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
                // Memory Use
                nextPage = 37;
            }
            if ((y >= 135) && (y <= 185))
            {
                waitForIt(140, 135, 305, 185);
                // Unused
                //nextPage = 39;
            }
            if ((y >= 190) && (y <= 240))
            {
                waitForIt(140, 190, 305, 240);
                // Unused
                //nextPage = 41;
            }
            if ((y >= 245) && (y <= 295))
            {
                waitForIt(140, 245, 305, 295);
                // Unused
                //nextPage = 43;
            }
        }
        if ((x >= 310) && (x <= 475))
        {
            if ((y >= 80) && (y <= 130))
            {
                waitForIt(310, 80, 475, 130);
                // Unused
                //nextPage = 38;
            }
            if ((y >= 135) && (y <= 185))
            {
                waitForIt(310, 135, 475, 185);
                // Unused
                //nextPage = 40;
            }
            if ((y >= 190) && (y <= 240))
            {
                waitForIt(310, 190, 475, 240);
                // Unused
                //nextPage = 42;
            }
            if ((y >= 245) && (y <= 295))
            {
                waitForIt(310, 245, 475, 295);
                //Unused
                //nextPage = 44;
            }
        }
    }
}

// Program to display used memory
void memoryUse()
{
    uint32_t MaxUsedHeapRAM = 0;
    uint32_t MaxUsedStackRAM = 0;
    uint32_t MaxUsedStaticRAM = 0;
    uint32_t MinfreeRAM = 0;
    saveRamStates(MaxUsedHeapRAM, MaxUsedStackRAM, MaxUsedStaticRAM, MinfreeRAM);
}