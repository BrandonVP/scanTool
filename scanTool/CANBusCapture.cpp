/*
 Name:    CANBusCapture.cpp
 Created: 11/15/2020 8:54:18 AM
 Author:  Brandon Van Pelt
*/

#pragma once

#include "CANBusCapture.h"
#include "common.h"
#include "definitions.h"

//#define DEBUG_FILTERMASK

/*=========================================================
	CAN Bus
===========================================================*/
// Draw CANBus menu buttons
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
		drawRoundBtn(140, 80, 305, 130, F("Capture"), menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
		break;
	case 3:
		drawRoundBtn(310, 80, 475, 130, F("Playback"), menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
		break;
	case 4:
		drawRoundBtn(140, 135, 305, 185, F("CAN0 RX"), menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
		break;
	case 5:
		drawRoundBtn(310, 135, 475, 185, F("CAN1 RX"), menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
		break;
	case 6:
		drawRoundBtn(140, 190, 305, 240, F("FilterMask"), menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
		break;
	case 7:
		drawRoundBtn(310, 190, 475, 240, F("Baud"), menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
		break;
	case 8:
		drawRoundBtn(140, 245, 305, 295, F(""), menuBackground, menuBtnBorder, menuBtnText, CENTER);
		break;
	case 9:
		drawRoundBtn(310, 245, 475, 295, F("Auto Baud"), menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
		break;
	case 10:
		drawSquareBtn(150, 301, 479, 319, VERSION, themeBackground, themeBackground, menuBtnColor, CENTER);
		break;
	}
}

// Buttons to start CANBus programs
void CANBusButtons()
{
	// Touch screen controls
	if (myTouch.dataAvailable())
	{
		myTouch.read();
		x = myTouch.getX();
		y = myTouch.getY();

		if ((x >= 140) && (x <= 308))
		{
			if ((y >= 80) && (y <= 130))
			{
				waitForIt(140, 80, 305, 130);
				// Capture
				nextPage = 1;
				state = 0;
				graphicLoaderState = 0;
			}
			if ((y >= 135) && (y <= 185))
			{
				waitForIt(140, 135, 305, 185);
				// CAN0 RX
				if (lockVar8(LOCK0))
				{
					graphicLoaderState = 0;
					nextPage = 3;
					g_var8[POS0] = 0;
				}
				else
				{
					// Error 
					DEBUG_ERROR("POS0 LOCKED");
				}
			}
			if ((y >= 190) && (y <= 240))
			{
				waitForIt(140, 190, 305, 240);
				// Filter Mask
				graphicLoaderState = 0;
				nextPage = 4;
			}
			if ((y >= 245) && (y <= 295))
			{
				//waitForIt(140, 245, 305, 295);
				// Unused
			}
		}
		if ((x >= 310) && (x <= 475))
		{
			if ((y >= 80) && (y <= 130))
			{
				waitForIt(310, 80, 475, 130);
				// Playback
				nextPage = 2;
			}
			if ((y >= 135) && (y <= 185))
			{
				waitForIt(310, 135, 475, 185);
				// CAN1 RX
				if (lockVar8(LOCK0))
				{
					graphicLoaderState = 0;
					nextPage = 3;
					g_var8[POS0] = 1;
				}
				else
				{
					// Error 
					DEBUG_ERROR("POS0 LOCKED");
				}
			}
			if ((y >= 190) && (y <= 240))
			{
				waitForIt(310, 190, 475, 240);
				// Baud
				nextPage = 5;
			}
			if ((y >= 245) && (y <= 295))
			{
				waitForIt(310, 245, 475, 295);
				// Find Baud
				findBaud();
			}
		}
	}
}

/*============== CAPTURE ==============*/
// Draw the stationary capture program buttons
void drawCapture()
{
	switch (graphicLoaderState)
	{
	case 0:
		break;
	case 1:
		drawSquareBtn(131, 55, 479, 319, "", themeBackground, themeBackground, themeBackground, CENTER);
		break;
	case 2:
		if (isSerialOut)
		{
			drawSquareBtn(310, 185, 470, 240, F("Start"), menuBackground, menuBtnBorder, menuBtnText, CENTER);
		}
		else
		{
			drawSquareBtn(310, 185, 470, 240, F("Start"), menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
		}
		break;
	case 3:
		if (isSerialOut)
		{
			drawSquareBtn(310, 245, 470, 300, F("Stop"), menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
		}
		else
		{
			drawSquareBtn(310, 245, 470, 300, F("Stop"), menuBackground, menuBtnBorder, menuBtnText, CENTER);
		}
		break;
	case 4:
		drawSquareBtn(150, 301, 479, 319, VERSION, themeBackground, themeBackground, menuBtnColor, CENTER);
		break;
	case 5:
		drawSquareBtn(310, 55, 475, 85, F("Selected"), menuBackground, menuBtnBorder, menuBtnText, CENTER);
		break;
	case 6:
		drawSquareBtn(310, 85, 475, 125, F(""), menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
		break;
	case 7:
		drawSquareBtn(310, 125, 475, 165, F(""), menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
		break;
	}
}

// Draw current selected capture configuration 
void drawCaptureSelected()
{
	switch (selectedChannelOut)
	{
	case 0:
		drawSquareBtn(310, 85, 475, 125, F(""), menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
		break;
	case 1:
		drawSquareBtn(310, 85, 475, 125, F("CAN0"), menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
		break;
	case 2:
		drawSquareBtn(310, 85, 475, 125, F("CAN1"), menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
		break;
	case 3:
		drawSquareBtn(310, 85, 475, 125, F("CAN0&1"), menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
		break;
	case 4:
		drawSquareBtn(310, 85, 475, 125, F("CAN0&TX1"), menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
		break;
	case 5:
		drawSquareBtn(310, 85, 475, 125, F("Bridge0&1"), menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
		break;
	}
	switch (selectedSourceOut)
	{
	case 0:
		drawSquareBtn(310, 125, 475, 165, F(""), menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
		break;
	case 1:
		drawSquareBtn(310, 125, 475, 165, F("LCD"), menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
		break;
	case 2:
		drawSquareBtn(310, 125, 475, 165, F("Serial"), menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
		break;
	case 3:
		drawSquareBtn(310, 125, 475, 165, F("SD Card"), menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
		break;
	}
}

// Draw capture menu
void drawCaptureOutput()
{
	switch (graphicLoaderState)
	{
	case 0:
		break;
	case 1:
		drawSquareBtn(135, 55, 300, 285, "", themeBackground, themeBackground, themeBackground, CENTER);
		break;
	case 2:
		drawSquareBtn(135, 55, 300, 85, F("Output"), menuBackground, menuBtnBorder, menuBtnText, CENTER);
		break;
	case 3:
		drawSquareBtn(135, 85, 300, 125, F("LCD"), menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
		break;
	case 4:
		drawSquareBtn(135, 125, 300, 165, F("Serial"), menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
		break;
	case 5:
		drawSquareBtn(135, 165, 300, 205, F("SD Card"), menuBackground, menuBtnBorder, menuBtnText, CENTER);
		break;
	case 6:
		drawSquareBtn(135, 205, 300, 245, F("Back"), menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
		break;
	}
}

// Draw source menu
void drawCaptureSource()
{
	switch (graphicLoaderState)
	{
	case 0:
		break;
	case 1:
		drawSquareBtn(135, 55, 300, 285, "", themeBackground, themeBackground, themeBackground, CENTER);
		break;
	case 2:
		drawSquareBtn(135, 55, 300, 85, F("Source"), menuBackground, menuBtnBorder, menuBtnText, CENTER);
		break;
	case 3:
		drawSquareBtn(135, 85, 300, 125, F("CAN0"), menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
		break;
	case 4:
		drawSquareBtn(135, 125, 300, 165, F("CAN1"), menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
		break;
	case 5:
		drawSquareBtn(135, 165, 300, 205, F("CAN0&1"), menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
		break;
	case 6:
		drawSquareBtn(135, 205, 300, 245, F("CAN0&TX1"), menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
		break;
	case 7:
		drawSquareBtn(135, 245, 300, 285, F("Bridge0&1"), menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
		break;
	}
}

// Buttons for capture program
void CaptureButtons()
{
	// Touch screen controls
	if (myTouch.dataAvailable())
	{
		myTouch.read();
		x = myTouch.getX();
		y = myTouch.getY();

		if ((x >= 135) && (x <= 300))
		{
			if ((y >= 85) && (y <= 125))
			{
				waitForIt(135, 85, 300, 125);
				if (state == 1)
				{
					// CAN0
					selectedChannelOut = 1;
					state = 2;
					hasDrawn = false;
					graphicLoaderState = 0;
				}
				else if (state == 2)
				{
					// LCD
					selectedSourceOut = 1;
					drawCaptureSelected();
				}
				drawCaptureSelected();
			}
			if ((y >= 125) && (y <= 165))
			{
				waitForIt(135, 125, 300, 165);
				if (state == 1)
				{
					// CAN1
					selectedChannelOut = 2;
					state = 2;
					hasDrawn = false;
					graphicLoaderState = 0;
				}
				else if (state == 2)
				{
					// Serial
					selectedSourceOut = 2;
					drawCaptureSelected();
				}
				drawCaptureSelected();
			}
			if ((y >= 165) && (y <= 205))
			{
				waitForIt(135, 165, 300, 205);
				if (state == 1)
				{
					// CAN0/1
					selectedChannelOut = 3;
					state = 2;
					hasDrawn = false;
					graphicLoaderState = 0;
				}
				else if (state == 2)
				{
					// SD Card
					selectedSourceOut = 3;
					drawCaptureSelected();
				}
				drawCaptureSelected();
			}
			if ((y >= 205) && (y <= 245))
			{
				waitForIt(135, 205, 300, 245);
				if (state == 1)
				{
					// CAN0/TX1
					selectedChannelOut = 4;
					state = 2;
					hasDrawn = false;
					graphicLoaderState = 0;
					drawCaptureSelected();
				}
				else if (state == 2)
				{
					// Back
					state = 1;
					hasDrawn = false;
					graphicLoaderState = 0;
				}
			}
			if ((y >= 245) && (y <= 285))
			{
				if (state == 1)
				{
					waitForIt(135, 245, 300, 285);
					// Bridge0/1
					selectedChannelOut = 5;
					state = 2;
					hasDrawn = false;
					graphicLoaderState = 0;
					drawCaptureSelected();
				}
				else if (state == 2)
				{
					// Unused
				}
			}
		}
		if ((x >= 310) && (x <= 470))
		{
			if ((y >= 200) && (y <= 240))
			{
				waitForIt(310, 185, 470, 240);
				// Start
				switch (selectedSourceOut)
				{
				case 1: // LCD
					nextPage = 7;
					g_var16[POS0] = 60;
					break;
				case 2: // Serial
					Can0.empty_rx_buff();
					Can1.empty_rx_buff();
					isSerialOut = true;
					drawSquareBtn(310, 185, 470, 240, F("Start"), menuBackground, menuBtnBorder, menuBtnText, CENTER);
					drawSquareBtn(310, 245, 470, 300, F("Stop"), menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
					break;
				case 3: // SD Card
					drawSquareBtn(310, 185, 470, 240, F("Start"), menuBackground, menuBtnBorder, menuBtnText, CENTER);
					drawSquareBtn(310, 245, 470, 300, F("Stop"), menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
					break;
				}
			}
			if ((y >= 245) && (y <= 285))
			{
				waitForIt(310, 245, 470, 300);
				// Stop
				isSerialOut = false;
				drawSquareBtn(310, 185, 470, 240, F("Start"), menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
				drawSquareBtn(310, 245, 470, 300, F("Stop"), menuBackground, menuBtnBorder, menuBtnText, CENTER);
			}
		}
	}
}

// Clears page before starting capture
void drawReadInCANLCD()
{
	drawSquareBtn(131, 55, 479, 319, "", themeBackground, themeBackground, themeBackground, CENTER);
}

// Capture to LCD, max rate without filling buffer is 26ms per message (slow)
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
		myGLCD.fillRect(150, (g_var16[POS0] - 5), 479, (g_var16[POS0] + 25));
		myGLCD.setColor(VGA_BLACK);
		sprintf(printString, "%04X  %d  %02X  %02X  %02X  %02X  %02X  %02X  %02X  %02X", rxId, len, rxBuf[0], rxBuf[1], rxBuf[2], rxBuf[3], rxBuf[4], rxBuf[5], rxBuf[6], rxBuf[7]);
		myGLCD.print(printString, 150, g_var16[POS0]);

		if (g_var16[POS0] < 300)
		{
			g_var16[POS0] += 15;
		}
		else
		{
			g_var16[POS0] = 60;
		}
	}
	myGLCD.setFont(BigFont);
}

/*============== Send Frame ==============*/
// Draw the send frame page
void drawSendFrame(uint8_t channel)
{
	switch (graphicLoaderState)
	{
	case 0:
		break;
	case 1:
		drawSquareBtn(131, 55, 479, 319, "", themeBackground, themeBackground, themeBackground, CENTER);
		break;
	case 2:
		drawRoundBtn(145, 55, 473, 95, F("ID"), menuBackground, menuBtnBorder, menuBtnText, CENTER);
		break;
	case 3:
		drawRoundBtn(145, 100, 473, 145, String(can1.getCANOutID(), 16), menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
		break;
	case 4:
		drawRoundBtn(145, 150, 473, 190, F("FRAME"), menuBackground, menuBtnBorder, menuBtnText, CENTER);
		break;
	case 5:
		myGLCD.setFont(SmallFont);
		drawRoundBtn(145, 195, 186, 250, " " + String(can1.getCANOutData(0), 16), menuBtnColor, menuBtnBorder, menuBtnText, LEFT);
		break;
	case 6:
		drawRoundBtn(186, 195, 227, 250, " " + String(can1.getCANOutData(1), 16), menuBtnColor, menuBtnBorder, menuBtnText, LEFT);
		break;
	case 7:
		drawRoundBtn(227, 195, 268, 250, " " + String(can1.getCANOutData(2), 16), menuBtnColor, menuBtnBorder, menuBtnText, LEFT);
		break;
	case 8:
		drawRoundBtn(268, 195, 309, 250, " " + String(can1.getCANOutData(3), 16), menuBtnColor, menuBtnBorder, menuBtnText, LEFT);
		break;
	case 9:
		drawRoundBtn(309, 195, 350, 250, " " + String(can1.getCANOutData(4), 16), menuBtnColor, menuBtnBorder, menuBtnText, LEFT);
		break;
	case 10:
		drawRoundBtn(350, 195, 391, 250, " " + String(can1.getCANOutData(5), 16), menuBtnColor, menuBtnBorder, menuBtnText, LEFT);
		break;
	case 11:
		drawRoundBtn(391, 195, 432, 250, " " + String(can1.getCANOutData(6), 16), menuBtnColor, menuBtnBorder, menuBtnText, LEFT);
		break;
	case 12:
		drawRoundBtn(432, 195, 473, 250, " " + String(can1.getCANOutData(7), 16), menuBtnColor, menuBtnBorder, menuBtnText, LEFT);
		myGLCD.setFont(BigFont);
		break;
	case 13:
		drawRoundBtn(145, 255, 473, 300, F("Send"), menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
		break;
	case 14:
		drawSquareBtn(150, 301, 479, 319, VERSION, themeBackground, themeBackground, menuBtnColor, CENTER);
		break;
	}
}

// Buttons for send frame program
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
		g_var16[POS1] = 1;
		g_var8[POS1] = 0xFF;
		g_var16[POS0] = 0;
		drawRoundBtn(145, 220, 470, 260, displayText, menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
	}

	// Keypad response
	g_var8[POS1] = keypadButtons();
	if (g_var8[POS1] >= 0x00 && g_var8[POS1] < 0x10 && g_var16[POS1] >= 0)
	{
		// current value + returned keypad value times its hex place
		g_var16[POS0] = g_var16[POS0] + (g_var8[POS1] * hexTable[g_var16[POS1]]);
		sprintf(displayText, "%02X", g_var16[POS0]);
		drawRoundBtn(145, 220, 470, 260, displayText, menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
		if (g_var16[POS1] >= 0)
		{
			g_var16[POS1]--;
		}
		g_var8[POS1] = 0xFF;
	}
	if (g_var8[POS1] == 0x10)
	{
		// Clear
		g_var16[POS1] = 1;
		g_var8[POS1] = 0;
		g_var16[POS0] = 0;
		drawRoundBtn(145, 220, 470, 260, F("0"), menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
	}
	if (g_var8[POS1] == 0x11)
	{
		// Accept
		can1.setDataCANOut(g_var16[POS0], position);
		isFinished = false;
		state = 0;
	}
	if (g_var8[POS1] == 0x12)
	{
		// Cancel
		isFinished = false;
		state = 0;
	}
}

// Send CAN Bus frame program
void sendCANFrame(uint8_t channel)
{
	char displayText[10];
	switch (state)
	{
	case 0: // Draw
		if (!isFinished)
		{
			if (graphicLoaderState < 15)
			{
				drawSendFrame(g_var8[POS0]);
				graphicLoaderState++;
				break;
			}
			graphicLoaderState = 0;
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
			g_var16[POS1] = 2;
			g_var8[POS1] = 0xFF;
			g_var32[POS0] = 0;
			drawRoundBtn(145, 220, 470, 260, displayText, menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
		}

		// Keypad response
		g_var8[POS1] = keypadButtons();
		if (g_var8[POS1] >= 0x00 && g_var8[POS1] < 0x10 && g_var16[POS1] >= 0)
		{
			// current value + returned keypad value times its hex place
			g_var32[POS0] = g_var32[POS0] + (g_var8[POS1] * hexTable[g_var16[POS1]]);
			sprintf(displayText, "%03X", g_var32[POS0]);
			drawRoundBtn(145, 220, 470, 260, displayText, menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
			if (g_var16[POS1] >= 0)
			{
				g_var16[POS1]--;
			}
			g_var8[POS1] = 0xFF;
		}
		if (g_var8[POS1] == 0x10)
		{
			g_var16[POS1] = 2;
			g_var8[POS1] = 0;
			g_var32[POS0] = 0;
			drawRoundBtn(145, 220, 470, 260, F("0"), menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
		}
		if (g_var8[POS1] == 0x11)
		{
			can1.setIDCANOut(g_var32[POS0]);
			drawRoundBtn(145, 100, 473, 145, String(can1.getCANOutID(), 16), menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
			isFinished = false;
			state = 0;
		}
		if (g_var8[POS1] == 0x12)
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

/*============== Baud ==============*/
// Draw the stationary baud page buttons
void drawBaud()
{
	drawSquareBtn(131, 55, 479, 319, "", themeBackground, themeBackground, themeBackground, CENTER);
	drawSquareBtn(140, 275, 300, 315, F("Set CAN0"), menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
	drawSquareBtn(305, 275, 475, 315, F("Set CAN1"), menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
	drawSquareBtn(305, 60, 475, 100, F("CAN0"), menuBackground, menuBtnBorder, menuBtnText, CENTER);
	drawSquareBtn(305, 150, 475, 190, F("CAN1"), menuBackground, menuBtnBorder, menuBtnText, CENTER);
}

// Draw the current baud rates
void drawCurrentBaud()
{
	drawSquareBtn(305, 100, 475, 140, String(can1.getBaud0()), menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
	drawSquareBtn(305, 190, 475, 230, String(can1.getBaud1()), menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
}

// Draw the list of baud rates
void drawBaudScroll()
{
	isWaitForIt = false;

	// Starting y location for list
	uint16_t y = 60;

	// Draw the scroll window
	for (uint8_t i = 0; i < MAXSCROLL; i++)
	{
		drawSquareBtn(140, y, 300, y + 35, String(baudRates[i]), menuBackground, menuBtnBorder, menuBtnText, CENTER);
		y = y + 35;
	}
}

// Buttons for the buad rate program
void baudButtons()
{
	// Touch screen controls
	if (myTouch.dataAvailable())
	{
		myTouch.read();
		x = myTouch.getX();
		y = myTouch.getY();

		if ((x >= 140) && (x <= 300))
		{
			if ((y >= 60) && (y <= 95))
			{
				waitForItRect(140, 60, 300, 95);
				var4 = baudRates[0];
			}
			if ((y >= 95) && (y <= 130))
			{
				waitForItRect(140, 95, 300, 130);
				var4 = baudRates[1];
			}
			if ((y >= 130) && (y <= 165))
			{
				waitForItRect(140, 130, 300, 165);
				var4 = baudRates[2];
			}
			if ((y >= 165) && (y <= 200))
			{
				waitForItRect(140, 165, 300, 200);
				var4 = baudRates[3];
			}
			if ((y >= 200) && (y <= 235))
			{
				waitForItRect(140, 200, 300, 235);
				var4 = baudRates[4];
			}
			if ((y >= 235) && (y <= 270))
			{
				waitForItRect(140, 235, 300, 270);
				var4 = baudRates[5];
			}
		}
		if ((y >= 275) && (y <= 315))
		{
			if ((x >= 140) && (x <= 300))
			{
				// CAN0
				waitForItRect(140, 275, 300, 315);
				can1.setBaud0(var4);
				drawCurrentBaud();
			}
			if ((x >= 305) && (x <= 465))
			{
				// CAN21
				waitForItRect(305, 275, 475, 315);
				can1.setBaud1(var4);
				drawCurrentBaud();
			}
		}
	}
}

/*============== Find Baud ==============*/
void findBaud()
{
	can1.setBaud0(can1.findBaudRate0());
	can1.setBaud1(can1.findBaudRate1());
}

/*============== Filter Mask ==============*/
bool drawFilterMask()
{
	switch (graphicLoaderState)
	{
	case 0:
		break;
	case 1:
		drawSquareBtn(131, 55, 479, 319, "", themeBackground, themeBackground, themeBackground, CENTER);
		break;
	case 2:
		drawRoundBtn(145, 70, 200, 120, F("CAN"), menuBackground, menuBtnBorder, menuBtnText, CENTER);
		break;
	case 3:
		drawRoundBtn(205, 70, 340, 120, F("Filter"), menuBackground, menuBtnBorder, menuBtnText, CENTER);
		break;
	case 4:
		drawRoundBtn(345, 70, 475, 120, F("Mask"), menuBackground, menuBtnBorder, menuBtnText, CENTER);
		break;
	case 5:
		drawRoundBtn(145, 125, 200, 175, F("0"), menuBackground, menuBtnBorder, menuBtnText, CENTER);
		break;
	case 6:
		drawRoundBtn(205, 125, 340, 175, String(CAN0Filter, 16), menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
		break;
	case 7:
		drawRoundBtn(345, 125, 475, 175, String(CAN0Mask, 16), menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
		break;
	case 8:
		drawRoundBtn(145, 180, 200, 230, F("1"), menuBackground, menuBtnBorder, menuBtnText, CENTER);
		break;
	case 9:
		drawRoundBtn(205, 180, 340, 230, String(CAN1Filter, 16), menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
		break;
	case 10:
		drawRoundBtn(345, 180, 475, 230, String(CAN1Mask, 16), menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
		break;
	case 11:
		drawRoundBtn(145, 235, 475, 285, F("Open All Traffic"), menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
		break;
	case 12:
		drawSquareBtn(150, 295, 479, 315, VERSION, themeBackground, themeBackground, menuBtnColor, CENTER);
		break;
	}
	graphicLoaderState++;
	return true;
}

void filterMaskButtons()
{
	// Touch screen controls
	if (myTouch.dataAvailable())
	{
		myTouch.read();
		x = myTouch.getX();
		y = myTouch.getY();

		if ((y >= 125) && (y <= 175))
		{
			if ((x >= 205) && (x <= 340))
			{
				waitForIt(205, 125, 340, 175);
				// Set CAN0 Filter
				state = 1;
				isFinished = false;
			}
			if ((x >= 345) && (x <= 475))
			{
				waitForIt(345, 125, 475, 175);
				// Set CAN 0Mask
				state = 2;
				isFinished = false;
			}
		}
		if ((y >= 180) && (y <= 230))
		{
			if ((x >= 205) && (x <= 340))
			{
				waitForIt(205, 180, 340, 230);
				// Set CAN1 Filter
				state = 3;
				isFinished = false;
			}
			if ((x >= 345) && (x <= 475))
			{
				waitForIt(345, 180, 475, 230);
				// Set CAN1 Mask
				state = 4;
				isFinished = false;
			}
		}
		if ((y >= 235) && (y <= 285))
		{
			if ((x >= 145) && (x <= 475))
			{
				waitForIt(145, 235, 475, 285);
				openAllTraffic();
				drawRoundBtn(205, 125, 340, 175, String(CAN0Filter, 16), menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
				drawRoundBtn(345, 125, 475, 175, String(CAN0Mask, 16), menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
				drawRoundBtn(205, 180, 340, 230, String(CAN1Filter, 16), menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
				drawRoundBtn(345, 180, 475, 230, String(CAN1Mask, 16), menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
			}
		}
	}
}

void openAllTraffic()
{
	CAN0Filter = 000;
	CAN0Mask = 0xFFF;
	CAN1Filter = 0x000;
	CAN1Mask = 0xFFF;
	can1.startCAN0(CAN0Filter, CAN0Mask);
	can1.startCAN1(CAN1Filter, CAN1Mask);
}

uint8_t setFilterMask(uint32_t& value)
{

	char displayText[10];
	if (!isFinished)
	{
		drawKeypad();
		isFinished = true;
		counter1 = 2;
		var4 = 0xFF;
		var5 = 0;
		drawRoundBtn(145, 220, 470, 260, F("000"), menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
	}

	var4 = keypadButtons();
	if (var4 >= 0x00 && var4 < 0x10 && counter1 >= 0)
	{
		// CAN0Filter = current value + returned keypad value times its hex place
		var5 = var5 + (var4 * hexTable[counter1]);
		sprintf(displayText, "%03X", var5);
		drawRoundBtn(145, 220, 470, 260, displayText, menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
		if (counter1 >= 0)
		{
			counter1--;
		}
		var4 = 0xFF;
	}
	if (var4 == 0x10)
	{
		counter1 = 2;
		var4 = 0;
		var5 = 0;
		sprintf(displayText, "%03X", var5);
		drawRoundBtn(145, 220, 470, 260, displayText, menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
	}
	if (var4 == 0x11)
	{
		isFinished = false;
		graphicLoaderState = 0;
		state = 0;
		value = var5;
		return 0x11;
	}
	if (var4 == 0x12)
	{
		isFinished = false;
		graphicLoaderState = 0;
		state = 0;
	}
	return 0;
}


void filterMask()
{
	uint8_t temp = 0;
	switch (state)
	{
	case 0:
		(!isFinished && graphicLoaderState < 13) ? drawFilterMask() : isFinished = true;
		filterMaskButtons();
		break;
	case 1: temp = setFilterMask(CAN0Filter);
		if (temp == 0x11)
		{
#if defined DEBUG_FILTERMASK
			DEBUG("\nFilter: ");
			DEBUG_HEX(CAN0Filter, 16);
			DEBUG("\nMask: ");
			DEBUG_HEX(CAN0Mask, 16);
			DEBUG("\n");
#endif
			can1.setFilterMask0(CAN0Filter, CAN0Mask);
			temp = 0;
		}
		break;
	case 2: temp = setFilterMask(CAN0Mask);
		if (temp == 0x11)
		{
#if defined DEBUG_FILTERMASK
			DEBUG("\nFilter: ");
			DEBUG_HEX(CAN0Filter, 16);
			DEBUG("\nMask: ");
			DEBUG_HEX(CAN0Mask, 16);
			DEBUG("\n");
#endif
			can1.setFilterMask0(CAN0Filter, CAN0Mask);
			temp = 0;
		}
		break;
	case 3: temp = setFilterMask(CAN1Filter);
		if (temp == 0x11)
		{
#if defined DEBUG_FILTERMASK
			DEBUG("\nFilter: ");
			DEBUG_HEX(CAN1Filter, 16);
			DEBUG("\nMask: ");
			DEBUG_HEX(CAN1Mask, 16);
			DEBUG("\n");
#endif
			can1.setFilterMask1(CAN1Filter, CAN1Mask);
			temp = 0;
		}
		break;
	case 4: temp = setFilterMask(CAN1Mask);
		if (temp == 0x11)
		{
#if defined DEBUG_FILTERMASK
			DEBUG("\nFilter: ");
			DEBUG_HEX(CAN1Filter, 16);
			DEBUG("\nMask: ");
			DEBUG_HEX(CAN1Mask, 16);
			DEBUG("\n");
#endif
			can1.setFilterMask1(CAN1Filter, CAN1Mask);
			temp = 0;
		}
		break;
	}
}

/*============== CAN Playback ==============*/
void drawCANLog()
{
	// clear LCD
	drawSquareBtn(131, 55, 479, 319, "", themeBackground, themeBackground, themeBackground, CENTER);

	drawSquareBtn(420, 80, 470, 160, F("/\\"), menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
	drawSquareBtn(420, 160, 470, 240, F("\\/"), menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
	drawSquareBtn(131, 275, 216, 315, F("Play"), menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
	drawSquareBtn(216, 275, 301, 315, F("Del"), menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
	drawSquareBtn(301, 275, 386, 315, F("Split"), menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
	drawSquareBtn(386, 275, 479, 315, F("Conf"), menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
}

//
void drawCANLogScroll()
{
	isWaitForIt = false;
	File root1;
	root1 = SD.open("/");

	uint8_t size = sdCard.printDirectory(root1, fileList);

	// Starting y location for list
	uint16_t y = 60;

	drawSquareBtn(131, 80, 415, 240, "", themeBackground, themeBackground, themeBackground, CENTER);

	// Draw the scroll window
	for (int i = 0; i < MAXSCROLL; i++)
	{
		if ((g_var8[POS0] + i < 10))
		{
			char temp[13];
			sprintf(temp, "%s", fileList[g_var8[POS0] + i]);
			drawSquareBtn(150, y, 410, y + 35, temp, menuBackground, menuBtnBorder, menuBtnText, LEFT);
		}
		else
		{
			drawSquareBtn(150, y, 410, y + 35, "", menuBackground, menuBtnBorder, menuBtnText, LEFT);
		}
		y = y + 35;
	}
}

//
void CANLogButtons()
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
				//SerialUSB.println(1 + scroll);
				g_var16[POS0] = 1 + scroll;
			}
			if ((y >= 95) && (y <= 130))
			{
				waitForItRect(150, 95, 410, 130);
				//SerialUSB.println(2 + scroll);
				g_var16[POS0] = 2 + scroll;
			}
			if ((y >= 130) && (y <= 165))
			{
				waitForItRect(150, 130, 410, 165);
				//SerialUSB.println(3 + scroll);
				g_var16[POS0] = 3 + scroll;
			}
			if ((y >= 165) && (y <= 200))
			{
				waitForItRect(150, 165, 410, 200);
				//SerialUSB.println(4 + scroll);
				g_var16[POS0] = 4 + scroll;
			}
			if ((y >= 200) && (y <= 235))
			{
				waitForItRect(150, 200, 410, 235);
				//SerialUSB.println(5 + scroll);
				g_var16[POS0] = 5 + scroll;
			}
			if ((y >= 235) && (y <= 270))
			{
				waitForItRect(150, 235, 410, 270);
				//SerialUSB.println(6 + scroll);
				g_var16[POS0] = 6 + scroll;
			}
		}
		if ((x >= 420) && (x <= 470))
		{
			if ((y >= 80) && (y <= 160))
			{
				waitForItRect(420, 80, 470, 160);
				if (scroll > 0)
				{
					scroll--;
					drawCANLogScroll();
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
					scroll++;
					drawCANLogScroll();
				}
			}
		}
		if ((y >= 275) && (y <= 315))
		{
			char fileLoc[20] = "CANLOG/";
			strcat(fileLoc, fileList[g_var16[POS0] - 1]);
			if ((x >= 131) && (x <= 216))
			{
				// Play
				waitForItRect(131, 275, 216, 315);
				sdCard.readLogFile(fileLoc);
			}
			if ((x >= 216) && (x <= 301))
			{
				// Delete
				waitForItRect(216, 275, 301, 315);
				drawErrorMSG(F("Confirmation"), F("Permenently"), F("Delete File?"));
				state = 1;
			}
			if ((x >= 301) && (x <= 386))
			{
				// Split
				waitForItRect(301, 275, 386, 315);
				uint32_t temp = sdCard.fileLength(fileLoc);
				sdCard.tempCopy(fileLoc);
				sdCard.split("canlog/temp.txt", temp);
				sdCard.deleteFile("canlog/temp.txt");
				drawCANLogScroll();
			}
			if ((x >= 386) && (x <= 479))
			{
				// Settings
				waitForItRect(386, 275, 479, 315);
				//sdCard.readLogFile(fileList[g_var16[POS0]]);
			}
		}
	}
}