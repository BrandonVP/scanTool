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
        drawRoundBtn(140, 190, 305, 240, F("WiFi Reset"), menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
        break;
    case 7:
        drawRoundBtn(310, 190, 475, 240, F("Clock"), menuBackground, menuBtnBorder, menuBtnText, CENTER);
        break;
    case 8:
        drawRoundBtn(140, 245, 305, 295, F("Dongle"), menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
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
        drawSquareBtn(135, 195, 479, 215, VERSION, themeBackground, themeBackground, menuBtnColor, CENTER);
        
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
    char serialBuffer[20];

    switch (graphicLoaderState)
    {
    case 0:
        break;
    case 1:
        drawSquareBtn(131, 55, 479, 319, "", themeBackground, themeBackground, themeBackground, CENTER);
        break;
    case 2:
        drawSquareBtn(135, 100, 479, 120, F("Wi-Fi Device"), themeBackground, themeBackground, menuBtnColor, CENTER);
        sprintf(serialBuffer, "%c%c%c%d%d", Serial3.read(), Serial3.read(), Serial3.read(), Serial3.read(), Serial3.read());
        drawSquareBtn(135, 125, 479, 145, serialBuffer, themeBackground, themeBackground, menuBtnColor, CENTER);
        break;
    case 3:
        drawSquareBtn(135, 165, 479, 185, F("MAC"), themeBackground, themeBackground, menuBtnColor, CENTER);
        sprintf(serialBuffer, "%X:%X:%X:%X:%X:%X", Serial3.read(), Serial3.read(), Serial3.read(), Serial3.read(), Serial3.read(), Serial3.read());
        drawSquareBtn(135, 190, 479, 210, serialBuffer, themeBackground, themeBackground, menuBtnColor, CENTER);
        break;
    case 4:
        drawSquareBtn(135, 230, 479, 250, F("Dongle MAC"), themeBackground, themeBackground, menuBtnColor, CENTER);
        sprintf(serialBuffer, "%X:%X:%X:%X:%X:%X", Serial3.read(), Serial3.read(), Serial3.read(), Serial3.read(), Serial3.read(), Serial3.read());
        drawSquareBtn(135, 255, 479, 275, serialBuffer, themeBackground, themeBackground, menuBtnColor, CENTER);
        break;
    case 5:
        Serial3.read(); // Ending byte
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
                
                // Clear Serial3 buffer
                while (Serial3.available())
                {
                    Serial3.read();
                }

                graphicLoaderState = 0;
                nextPage = 39;
            }
            if ((y >= 190) && (y <= 240))
            {
                waitForIt(140, 190, 305, 240);
                // WiFi Reset
                Serial3.write(0xBA);
                Serial3.write(0xBF);
                if (Serial3.read() == 0xFF);
                // Confirmation
            }
            if ((y >= 245) && (y <= 295))
            {
                waitForIt(140, 245, 305, 295);
				graphicLoaderState = 0;
				nextPage = 43;
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
            }
            if ((y >= 190) && (y <= 240))
            {
                //waitForIt(310, 190, 475, 240);
				// Unused
            }
            if ((y >= 245) && (y <= 295))
            {
                waitForIt(310, 245, 475, 295);
                // Unused
            }
        }
    }
}

/*============== Connect Dongle ==============*/
savedMACs dongle;
uint8_t displayedMACNodePosition[5];

void loadMACs()
{
	sdCard.readMAC(dongle);
}

void saveMACs()
{
	sdCard.writeMACs(dongle);
}

bool drawMACNode(uint8_t index)
{
	g_var8[POS4] = 0;
	uint16_t yAxis = 55;
	for (uint8_t i = index; i < (index + 5); i++)
	{
		if (!dongle.node[i].isDel && i < 10)
		{
			drawSquareBtn(133, yAxis, 271, yAxis + 40, dongle.node[i].name, menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
			dongle.node[i].isOn ? drawSquareBtn(271, yAxis, 340, yAxis + 40, F("ON"), menuBtnColor, menuBtnBorder, menuBtnText, CENTER) : drawSquareBtn(271, yAxis, 340, yAxis + 40, F("OFF"), menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
			drawSquareBtn(340, yAxis, 409, yAxis + 40, F("EDIT"), menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
			drawSquareBtn(409, yAxis, 477, yAxis + 40, F("DEL"), menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
			displayedMACNodePosition[g_var8[POS4]] = i;
			g_var8[POS4]++;
			yAxis += 45;
		}
		else if (i < 10)
		{
			index++;
		}
		// Nothing left to print fill space with blanks
		else if (i > 8)
		{
			drawSquareBtn(132, yAxis, 478, yAxis + 40, F(""), menuBackground, menuBtnBorder, menuBtnText, CENTER);
			yAxis += 45;
		}
		backgroundProcess();
	}
}

bool drawMACList()
{
	switch (graphicLoaderState)
	{
	case 0:
		break;
	case 1:
		drawSquareBtn(131, 55, 479, 319, "", themeBackground, themeBackground, themeBackground, CENTER);
		break;
	case 3:
		drawSquareBtn(132, 56, 478, 96, F(""), menuBackground, menuBtnBorder, menuBtnText, CENTER);
		break;
	case 4:
		break;
	case 5:
		break;
	case 6:
		break;
	case 7:
		break;
	case 8:
		drawSquareBtn(132, 55, 478, 96, F(""), menuBackground, menuBtnBorder, menuBtnText, CENTER);
		break;
	case 9:
		drawSquareBtn(132, 99, 478, 141, F(""), menuBackground, menuBtnBorder, menuBtnText, CENTER);
		break;
	case 10:
		drawSquareBtn(132, 144, 478, 186, F(""), menuBackground, menuBtnBorder, menuBtnText, CENTER);
		break;
	case 11:
		drawSquareBtn(132, 189, 478, 231, F(""), menuBackground, menuBtnBorder, menuBtnText, CENTER);
		break;
	case 12:
		drawSquareBtn(132, 279, 211, 318, F(""), menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
		break;
	case 13:
		drawSquareBtn(132, 234, 478, 276, F(""), menuBackground, menuBtnBorder, menuBtnText, CENTER);
		break;
	case 14:
		drawSquareBtn(133, 280, 210, 317, F("/\\"), menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
		break;
	case 15:
		drawSquareBtn(260, 279, 351, 318, F(""), menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
		break;
	case 16:
		drawSquareBtn(261, 280, 350, 317, F("Add"), menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
		break;
	case 17:
		drawSquareBtn(400, 279, 478, 318, F(""), menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
		break;
	case 18:
		drawSquareBtn(401, 280, 477, 317, F("\\/"), menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
		break;
	case 19:
		drawMACNode(g_var8[POS3]);
		break;
	case 20:
		return false;
		break;
	}
	return true;
}

bool drawEditMACNode(uint8_t node)
{
	switch (graphicLoaderState)
	{
	case 0:
		break;
	case 1:
		drawSquareBtn(131, 55, 479, 319, "", themeBackground, themeBackground, themeBackground, CENTER);
		break;
	case 3:
		drawRoundBtn(135, 55, 475, 95, F("Device Name"), menuBackground, menuBtnBorder, menuBtnText, CENTER);
		break;
	case 4:
		drawRoundBtn(135, 100, 475, 140, dongle.node[node].name, menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
		break;
	case 5:
		break;
	case 6:
		break;
	case 7:
		break;
	case 8:
		break;
	case 9:
		break;
	case 10:
		drawRoundBtn(135, 160, 475, 200, F("MAC"), menuBackground, menuBtnBorder, menuBtnText, CENTER);
		break;
	case 11:
		myGLCD.setFont(SmallFont);
		drawRoundBtn(135, 205, 190, 250, " " + String(dongle.node[g_var8[POS0]].data[0], 16), menuBtnColor, menuBtnBorder, menuBtnText, LEFT);
		break;
	case 12:
		drawRoundBtn(192, 205, 247, 250, " " + String(dongle.node[g_var8[POS0]].data[1], 16), menuBtnColor, menuBtnBorder, menuBtnText, LEFT);
		break;
	case 13:
		drawRoundBtn(249, 205, 304, 250, " " + String(dongle.node[g_var8[POS0]].data[2], 16), menuBtnColor, menuBtnBorder, menuBtnText, LEFT);
		break;
	case 14:
		drawRoundBtn(306, 205, 361, 250, " " + String(dongle.node[g_var8[POS0]].data[3], 16), menuBtnColor, menuBtnBorder, menuBtnText, LEFT);
		break;
	case 15:
		drawRoundBtn(363, 205, 418, 250, " " + String(dongle.node[g_var8[POS0]].data[4], 16), menuBtnColor, menuBtnBorder, menuBtnText, LEFT);
		break;
	case 16:
		drawRoundBtn(420, 205, 475, 250, " " + String(dongle.node[g_var8[POS0]].data[5], 16), menuBtnColor, menuBtnBorder, menuBtnText, LEFT);
		myGLCD.setFont(BigFont);
		break;
	case 17:
		break;
	case 18:
		
		break;
	case 19:
		drawRoundBtn(135, 276, 304, 316, F("Accept"), menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
		break;
	case 20:
		drawRoundBtn(306, 276, 475, 316, F("Cancel"), menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
		break;
	case 21:
		//drawMACNode(dongle.node[g_var8[POS0]].channel);
		return false;
		break;
	}
	graphicLoaderState++;
	return true;
}

void deleteMACNode(uint8_t node)
{
	for (uint8_t i = 0; i < 9; i++)
	{
		dongle.node[node].name[i] = '\0';
	}
	dongle.node[node].isDel = true;
	dongle.node[node].isOn = false;
	for (uint8_t i = 0; i < 6; i++)
	{
		dongle.node[node].data[i] = 0;
	}
}

void serialConnect(uint8_t index)
{
	// TODO: Set all other node to off
	for (uint8_t i = 0; i < 10; i++)
	{
		dongle.node[i].isOn = false;
	}
	dongle.node[index].isOn = true;

	Serial3.write(0xCA);
	Serial3.write(0xCC);
	Serial3.write(dongle.node[index].data[0]);
	Serial3.write(dongle.node[index].data[1]);
	Serial3.write(dongle.node[index].data[2]);
	Serial3.write(dongle.node[index].data[3]);
	Serial3.write(dongle.node[index].data[4]);
	Serial3.write(dongle.node[index].data[5]);
	delay(100);
	// WiFi Reset
	Serial3.write(0xBA);
	Serial3.write(0xBF);
	if (Serial3.read() == 0xFF);
	delay(100);
	//REQUEST_EXTERNAL_RESET;
}

void dongleButtons()
{
	if (Touch_getXY())
	{
		if ((y >= 55) && (y <= 95) && g_var8[POS4] > 0)
		{
			if ((x >= 133) && (x <= 340))
			{
				waitForIt(133, 55, 340, 95);
				// On
				if (!dongle.node[displayedMACNodePosition[0]].isOn)
				{
					dongle.node[displayedMACNodePosition[0]].isOn = true;
					serialConnect(displayedMACNodePosition[0]);
					drawMACNode(g_var8[POS3]);
				}
			}
			if ((x >= 340) && (x <= 409))
			{
				waitForIt(340, 55, 409, 95);
				// Edit
				graphicLoaderState = 0;
				g_var8[POS0] = displayedMACNodePosition[0];
				state = 2;
			}
			if ((x >= 409) && (x <= 477))
			{
				waitForIt(409, 55, 477, 95);
				// Del
				deleteMACNode(displayedMACNodePosition[0]);
				saveMACs();
				drawMACNode(g_var8[POS3]);
			}
		}
		if ((y >= 100) && (y <= 140) && g_var8[POS4] > 1)
		{
			if ((x >= 133) && (x <= 340))
			{
				waitForIt(133, 100, 340, 140);
				// On
				if (!dongle.node[displayedMACNodePosition[1]].isOn)
				{
					dongle.node[displayedMACNodePosition[1]].isOn = true;
					serialConnect(displayedMACNodePosition[1]);
					drawMACNode(g_var8[POS3]);
				}
			}
			if ((x >= 340) && (x <= 409))
			{
				waitForIt(340, 100, 409, 140);
				// Edit
				graphicLoaderState = 0;
				g_var8[POS0] = displayedMACNodePosition[1];
				state = 2;
			}
			if ((x >= 409) && (x <= 477))
			{
				waitForIt(409, 100, 477, 140);
				// Del
				deleteMACNode(displayedMACNodePosition[1]);
				saveMACs();
				drawMACNode(g_var8[POS3]);
			}
		}
		if ((y >= 145) && (y <= 185) && g_var8[POS4] > 2)
		{
			if ((x >= 133) && (x <= 340))
			{
				waitForIt(133, 145, 340, 185);
				// On
				if (!dongle.node[displayedMACNodePosition[2]].isOn)
				{
					dongle.node[displayedMACNodePosition[2]].isOn = true;
					serialConnect(displayedMACNodePosition[2]);
					drawMACNode(g_var8[POS3]);
				}
			}
			if ((x >= 340) && (x <= 409))
			{
				waitForIt(340, 145, 409, 185);
				// Edit
				graphicLoaderState = 0;
				g_var8[POS0] = displayedMACNodePosition[2];
				state = 2;
			}
			if ((x >= 409) && (x <= 477))
			{
				waitForIt(409, 145, 477, 185);
				// Del
				deleteMACNode(displayedMACNodePosition[2]);
				saveMACs();
				drawMACNode(g_var8[POS3]);
			}
		}
		if ((y >= 190) && (y <= 230) && g_var8[POS4] > 3)
		{
			if ((x >= 133) && (x <= 340))
			{
				waitForIt(133, 190, 340, 230);
				// On
				if (!dongle.node[displayedMACNodePosition[3]].isOn)
				{
					dongle.node[displayedMACNodePosition[3]].isOn = true;
					serialConnect(displayedMACNodePosition[3]);
					drawMACNode(g_var8[POS3]);
				}
			}
			if ((x >= 340) && (x <= 409))
			{
				waitForIt(340, 190, 409, 230);
				// Edit
				graphicLoaderState = 0;
				g_var8[POS0] = displayedMACNodePosition[3];
				state = 2;
			}
			if ((x >= 409) && (x <= 477))
			{
				waitForIt(409, 190, 477, 230);
				// Del
				deleteMACNode(displayedMACNodePosition[3]);
				saveMACs();
				drawMACNode(g_var8[POS3]);
			}
		}
		if ((y >= 235) && (y <= 275) && g_var8[POS4] > 4)
		{
			if ((x >= 133) && (x <= 340))
			{
				waitForIt(133, 235, 340, 275);
				// On
				if (!dongle.node[displayedMACNodePosition[4]].isOn)
				{
					dongle.node[displayedMACNodePosition[4]].isOn = true;
					serialConnect(displayedMACNodePosition[4]);
					drawMACNode(g_var8[POS3]);
				}
			}
			if ((x >= 340) && (x <= 409))
			{
				waitForIt(340, 235, 409, 275);
				// Edit
				graphicLoaderState = 0;
				g_var8[POS0] = displayedMACNodePosition[4];
				state = 2;
			}
			if ((x >= 409) && (x <= 477))
			{
				waitForIt(409, 235, 477, 275);
				// Del
				deleteMACNode(displayedMACNodePosition[4]);
				saveMACs();
				drawMACNode(g_var8[POS3]);
			}
		}
		if ((y >= 280) && (y <= 317))
		{
			if ((x >= 133) && (x <= 210))
			{
				waitForIt(133, 280, 210, 317);
				// Up
				if (g_var8[POS3] > 0)
				{
					g_var8[POS3]--;
					drawMACNode(g_var8[POS3]);
				}
			}
			if ((x >= 261) && (x <= 350))
			{
				waitForIt(261, 280, 350, 317);
				// Add
				graphicLoaderState = 0;
				state = 1;
			}
			if ((x >= 401) && (x <= 477))
			{
				waitForIt(401, 280, 477, 317);
				// Down
				if (g_var8[POS3] < 15)
				{
					g_var8[POS3]++;
					drawMACNode(g_var8[POS3]);
				}
			}
		}
	}
}

void editMACNodeButtons()
{
	// Touch screen controls
	if (Touch_getXY())
	{
		if ((y >= 100) && (y <= 140))
		{
			if ((x >= 135) && (x <= 475))
			{
				waitForIt(135, 100, 475, 140);
				// Set name
				g_var8[POS5] = 0;
				drawkeyboard();
				state = 14;
			}
		}
		if ((y >= 143) && (y <= 183))
		{
			if ((x >= 182) && (x <= 245))
			{
				waitForIt(182, 143, 245, 183);
				// Set ID
				g_var8[POS2] = 0;
				g_var16[POS0] = 0;
				resetKeypad();
				drawKeypad();
				state = 4;
			}
			if ((x >= 380) && (x <= 475))
			{
				waitForIt(380, 143, 475, 183);
				// Set Interval
				g_var8[POS2] = 0;
				g_var16[POS0] = 0;
				resetKeypad();
				drawKeypadDec();
				state = 5;
			}
		}
		if ((y >= 205) && (y <= 250))
		{
			if ((x >= 135) && (x <= 190))
			{
				waitForIt(135, 205, 190, 250);
				// Set Data[0]
				g_var8[POS2] = 0;
				g_var16[POS0] = 0;
				resetKeypad();
				state = 6;
				drawKeypad();
			}
			if ((x >= 192) && (x <= 247))
			{
				waitForIt(192, 205, 247, 250);
				// Set Data[1]
				g_var8[POS2] = 0;
				g_var16[POS0] = 0;
				resetKeypad();
				state = 7;
				drawKeypad();
			}
			if ((x >= 249) && (x <= 304))
			{
				waitForIt(249, 205, 304, 250);
				// Set Data[2]
				g_var8[POS2] = 0;
				g_var16[POS0] = 0;
				resetKeypad();
				state = 8;
				drawKeypad();
			}
			if ((x >= 306) && (x <= 361))
			{
				waitForIt(306, 205, 361, 250);
				// Set Data[3]
				g_var8[POS2] = 0;
				g_var16[POS0] = 0;
				resetKeypad();
				state = 9;
				drawKeypad();
			}
			if ((x >= 363) && (x <= 418))
			{
				waitForIt(363, 205, 418, 250);
				// Set Data[4]
				g_var8[POS2] = 0;
				g_var16[POS0] = 0;
				resetKeypad();
				state = 10;
				drawKeypad();
			}
			if ((x >= 420) && (x <= 475))
			{
				waitForIt(420, 205, 475, 250);
				g_var8[POS2] = 0;
				g_var16[POS0] = 0;
				resetKeypad();
				// Set Data[5]
				state = 11;
				drawKeypad();
			}
		}
		if ((y >= 276) && (y <= 316))
		{
			if ((x >= 135) && (x <= 304))
			{
				waitForIt(135, 276, 304, 316);
				// Accept
				graphicLoaderState = 0;
				dongle.node[g_var8[POS0]].isDel = false;
				saveMACs();
				state = 0;
			}
			if ((x >= 306) && (x <= 475))
			{
				waitForIt(306, 276, 475, 316);
				// Cancel
				graphicLoaderState = 0;
				state = 0;
			}
		}
	}
}

uint8_t findFreeMACNode()
{
	for (uint8_t i = 0; i < TIMED_TX_MAX_SIZE; i++)
	{
		if (dongle.node[i].isDel)
		{
			return i;
		}
	}
	return 0xFF;
}

void connectDongle()
{
	switch (state)
	{
	case 0:
		if (drawMACList())
		{
			graphicLoaderState++;
		}
		dongleButtons();
		break;
	case 1:
		// Add state
		g_var8[POS0] = findFreeMACNode();
		// 0xFF = no free slots
		(g_var8[POS0] == 0xFF) ? state = 0 : state = 2;
		break;
	case 2:
		drawEditMACNode(g_var8[POS0]) ? state = 2 : state = 3;
		break;
	case 3:
		editMACNodeButtons();
		break;
	case 4:
		//ID
		g_var8[POS1] = keypadController(g_var8[POS2], g_var16[POS0]);
		if (g_var8[POS1] == 0xF1) // Accept
		{
			graphicLoaderState = 0;
			//dongle.node[g_var8[POS0]].id = g_var16[POS0];
			state = 2;
		}
		else if (g_var8[POS1] == 0xF0) // Cancel
		{
			graphicLoaderState = 0;
			state = 2;
		}
		break;
	case 5:
		// INT
		g_var8[POS1] = keypadControllerDec(g_var8[POS2], g_var16[POS0]);
		if (g_var8[POS1] == 0xF1) // Accept
		{
			graphicLoaderState = 0;
			//dongle.node[g_var8[POS0]].interval = g_var16[POS0];
			state = 2;
		}
		else if (g_var8[POS1] == 0xF0) // Cancel
		{
			graphicLoaderState = 0;
			state = 2;
		}
		break;
	case 6:
		// Data0
		g_var8[POS1] = keypadController(g_var8[POS2], g_var16[POS0]);
		if (g_var8[POS1] == 0xF1) // Accept
		{
			graphicLoaderState = 0;
			dongle.node[g_var8[POS0]].data[0] = g_var16[POS0];
			state = 2;
		}
		else if (g_var8[POS1] == 0xF0) // Cancel
		{
			graphicLoaderState = 0;
			state = 2;
		}
		break;
	case 7:
		// Data1
		g_var8[POS1] = keypadController(g_var8[POS2], g_var16[POS0]);
		if (g_var8[POS1] == 0xF1) // Accept
		{
			graphicLoaderState = 0;
			dongle.node[g_var8[POS0]].data[1] = g_var16[POS0];
			state = 2;
		}
		else if (g_var8[POS1] == 0xF0) // Cancel
		{
			graphicLoaderState = 0;
			state = 2;
		}
		break;
	case 8:
		// Data2
		g_var8[POS1] = keypadController(g_var8[POS2], g_var16[POS0]);
		if (g_var8[POS1] == 0xF1) // Accept
		{
			graphicLoaderState = 0;
			dongle.node[g_var8[POS0]].data[2] = g_var16[POS0];
			state = 2;
		}
		else if (g_var8[POS1] == 0xF0) // Cancel
		{
			graphicLoaderState = 0;
			state = 2;
		}
		break;
	case 9:
		// Data3
		g_var8[POS1] = keypadController(g_var8[POS2], g_var16[POS0]);
		if (g_var8[POS1] == 0xF1) // Accept
		{
			graphicLoaderState = 0;
			dongle.node[g_var8[POS0]].data[3] = g_var16[POS0];
			state = 2;
		}
		else if (g_var8[POS1] == 0xF0) // Cancel
		{
			graphicLoaderState = 0;
			state = 2;
		}
		break;
	case 10:
		// Data4
		g_var8[POS1] = keypadController(g_var8[POS2], g_var16[POS0]);
		if (g_var8[POS1] == 0xF1) // Accept
		{
			graphicLoaderState = 0;
			dongle.node[g_var8[POS0]].data[4] = g_var16[POS0];
			state = 2;
		}
		else if (g_var8[POS1] == 0xF0) // Cancel
		{
			graphicLoaderState = 0;
			state = 2;
		}
		break;
	case 11:
		// Data5
		g_var8[POS1] = keypadController(g_var8[POS2], g_var16[POS0]);
		if (g_var8[POS1] == 0xF1) // Accept
		{
			graphicLoaderState = 0;
			dongle.node[g_var8[POS0]].data[5] = g_var16[POS0];
			state = 2;
		}
		else if (g_var8[POS1] == 0xF0) // Cancel
		{
			graphicLoaderState = 0;
			state = 2;
		}
		break;
	case 12:
		// Data6
		g_var8[POS1] = keypadController(g_var8[POS2], g_var16[POS0]);
		if (g_var8[POS1] == 0xF1) // Accept
		{
			graphicLoaderState = 0;
			dongle.node[g_var8[POS0]].data[6] = g_var16[POS0];
			state = 2;
		}
		else if (g_var8[POS1] == 0xF0) // Cancel
		{
			graphicLoaderState = 0;
			state = 2;
		}
		break;
	case 13:
		// Data7
		g_var8[POS1] = keypadController(g_var8[POS2], g_var16[POS0]);
		if (g_var8[POS1] == 0xF1) // Accept
		{
			graphicLoaderState = 0;
			dongle.node[g_var8[POS0]].data[7] = g_var16[POS0];
			state = 2;
		}
		else if (g_var8[POS1] == 0xF0) // Cancel
		{
			graphicLoaderState = 0;
			state = 2;
		}
		break;
	case 14:
		// Name
		g_var8[POS1] = keyboardController(g_var8[POS5]);
		if (g_var8[POS1] == 0xF1) // Accept
		{
			graphicLoaderState = 0;
			strncpy(dongle.node[g_var8[POS0]].name, keyboardInput, 9);
			for (uint8_t i = 0; i < 8; i++)
			{
				keyboardInput[i] = '\0';
			}
			state = 2;
		}
		else if (g_var8[POS1] == 0xF0) // Cancel
		{
			graphicLoaderState = 0;
			state = 2;
		}
		break;
	}
}

/*============== Memory Use ==============*/
// Program to display used memory
void memoryUse()
{
    uint32_t MaxUsedHeapRAM = 0;
    uint32_t MaxUsedStackRAM = 0;
    uint32_t MaxUsedStaticRAM = 0;
    uint32_t MinfreeRAM = 0;
    saveRamStates(MaxUsedHeapRAM, MaxUsedStackRAM, MaxUsedStaticRAM, MinfreeRAM);
}