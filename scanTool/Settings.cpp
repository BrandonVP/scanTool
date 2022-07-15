// 
// 
// 

#include "Settings.h"
#include "common.h"
#include "definitions.h"

// https://forum.arduino.cc/t/due-software-reset/332764/5
//Defines so the device can do a self reset
#define SYSRESETREQ    (1<<2)
#define VECTKEY        (0x05fa0000UL)
#define VECTKEY_MASK   (0x0000ffffUL)
#define AIRCR          (*(uint32_t*)0xe000ed0cUL) // fixed arch-defined address
#define REQUEST_EXTERNAL_RESET (AIRCR=(AIRCR&VECTKEY_MASK)|VECTKEY|SYSRESETREQ)

/*=========================================================
    Settings
===========================================================*/
// Draw setting page
bool drawSettings()
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
        drawRoundBtn(310, 80, 475, 130, F("About"), menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
        break;
    case 4:
        drawRoundBtn(140, 135, 305, 185, F("WiFi MAC"), menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
        break;
    case 5:
        drawRoundBtn(310, 135, 475, 185, F("Reset"), menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
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
    return true;
}

bool drawAbout()
{
    switch (graphicLoaderState)
    {
    case 0:
        break;
    case 1:
        drawSquareBtn(131, 55, 479, 319, "", themeBackground, themeBackground, themeBackground, CENTER);
        break;
    case 2:
        drawSquareBtn(135, 120, 479, 140, F("Software Development"), themeBackground, themeBackground, menuBtnColor, CENTER);
        drawSquareBtn(135, 145, 479, 165, F("Brandon Van Pelt"), themeBackground, themeBackground, menuBtnColor, CENTER);
        drawSquareBtn(135, 170, 479, 190, F("github.com/BrandonVP"), themeBackground, themeBackground, menuBtnColor, CENTER);
        break;
    case 3:

        break;
    case 4:

        break;
    case 5:

        return false;
    }
    graphicLoaderState++;
    return true;
}

bool drawMACAddress()
{
    switch (graphicLoaderState)
    {
    case 0:
        break;
    case 1:
        drawSquareBtn(131, 55, 479, 319, "", themeBackground, themeBackground, themeBackground, CENTER);
        break;
    case 2:
        drawRoundBtn(140, 80, 305, 130, F("Mac Address"), menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
        break;
    case 3:

        break;
    case 4:

        break;
    case 5:

        break;
    case 6:

        return false;
    }
    graphicLoaderState++;
    return true;
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
                // WiFi MAC Address
                graphicLoaderState = 0;
                nextPage = 39;
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
                // About
                graphicLoaderState = 0;
                nextPage = 38;
            }
            if ((y >= 135) && (y <= 185))
            {
                waitForIt(310, 135, 475, 185);
                // Reset
                REQUEST_EXTERNAL_RESET;
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