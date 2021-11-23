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
bool drawCANBus()
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
		drawRoundBtn(140, 135, 305, 185, F("Send"), menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
		break;
	case 5:
		drawRoundBtn(310, 135, 475, 185, F("Timed TX"), menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
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
	case 11:
		return false;
		break;
	}
	return true;
}

// Buttons to start CANBus programs
void CANBusButtons()
{
	// Touch screen controls
	if (Touch_getXY())
	{
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
				graphicLoaderState = 0;
				nextPage = 3;
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
				graphicLoaderState = 0;
				nextPage = 2;
			}
			if ((y >= 135) && (y <= 185))
			{
				waitForIt(310, 135, 475, 185);
				// Timed TX
				graphicLoaderState = 0;
				nextPage = 6;
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
bool drawCapture()
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
		//drawSquareBtn(150, 301, 479, 319, VERSION, themeBackground, themeBackground, menuBtnColor, CENTER);
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
	case 8:
		return false;
		break;
	}
	return true;
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
	case 6:
		drawSquareBtn(310, 85, 475, 125, F("WIFI"), menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
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
bool drawCaptureOutput()
{
	switch (graphicLoaderState)
	{
	case 0:
		break;
	case 1:
		drawSquareBtn(135, 55, 300, 320, "", themeBackground, themeBackground, themeBackground, CENTER);
		break;
	case 2:
		drawSquareBtn(135, 55, 300, 85, F("Output"), menuBackground, menuBtnBorder, menuBtnText, CENTER);
		break;
	case 3:
		selectedChannelOut == 6 ? drawSquareBtn(135, 85, 300, 125, F("LCD"), menuBackground, menuBtnBorder, menuBtnText, CENTER) : drawSquareBtn(135, 85, 300, 125, F("LCD"), menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
		break;
	case 4:
		drawSquareBtn(135, 125, 300, 165, F("Serial"), menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
		break;
	case 5:
		drawSquareBtn(135, 165, 300, 205, F("SD Card"), menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
		break;
	case 6:
		drawSquareBtn(135, 205, 300, 245, F("Back"), menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
		break;
	case 7:
		return false;
		break;
	}
	return true;
}

// Draw source menu
bool drawCaptureSource()
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
	case 8:
		drawSquareBtn(135, 285, 300, 320, F("WIFI"), menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
		break;
	case 9:
		return false;
		break;
	}
	return true;
}

// Buttons for capture program
void CaptureButtons()
{
	// Touch screen controls
	if (Touch_getXY())
	{
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
				else if (state == 2 && selectedChannelOut != 6) // LCD wont work with WIFI
				{
					// LCD
					selectedSourceOut = 1;
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
			if ((y >= 285) && (y <= 320))
			{
				if (state == 1)
				{
					waitForIt(135, 285, 300, 320);
					// WIFI
					selectedChannelOut = 6;
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
					can1.incCaptureFile();
					Can0.empty_rx_buff();
					Can1.empty_rx_buff();
					isSDOut = true;
					drawSquareBtn(310, 185, 470, 240, F("Start"), menuBackground, menuBtnBorder, menuBtnText, CENTER);
					drawSquareBtn(310, 245, 470, 300, F("Stop"), menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
					sdCard.writeFile("capture.txt", "");
					break;
				}
			}
			if ((y >= 245) && (y <= 285))
			{
				waitForIt(310, 245, 470, 300);
				// Stop
				isSerialOut = false;
				isSDOut = false;
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

// Capture to LCD, max rate without filling buffer is 26ms per message (slow), useful for bench testings or filtered traffic
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
bool drawSendFrame(uint8_t channel)
{
	switch (graphicLoaderState)
	{
	case 0:
		break;
	case 1:
		drawSquareBtn(131, 55, 479, 319, "", themeBackground, themeBackground, themeBackground, CENTER);
		break;
	case 3:
		drawRoundBtn(135, 55, 259, 95, F("Channel"), menuBackground, menuBtnBorder, menuBtnText, CENTER);
		break;
	case 4:
		drawRoundBtn(135, 100, 259, 145, F("ID:"), menuBackground, menuBtnBorder, menuBtnText, CENTER);
		break;
	case 5:
		drawRoundBtn(261, 100, 475, 145, String(can1.getCANOutID(), 16), menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
		break;
	case 6:
		drawRoundBtn(135, 150, 475, 190, F("FRAME"), menuBackground, menuBtnBorder, menuBtnText, CENTER);
		break;
	case 7:
		myGLCD.setFont(SmallFont);
		drawRoundBtn(134, 195, 175, 250, " " + String(can1.getCANOutData(0), 16), menuBtnColor, menuBtnBorder, menuBtnText, LEFT);
		break;
	case 8:
		drawRoundBtn(177, 195, 218, 250, " " + String(can1.getCANOutData(1), 16), menuBtnColor, menuBtnBorder, menuBtnText, LEFT);
		break;
	case 9:
		drawRoundBtn(220, 195, 261, 250, " " + String(can1.getCANOutData(2), 16), menuBtnColor, menuBtnBorder, menuBtnText, LEFT);
		break;
	case 10:
		drawRoundBtn(263, 195, 304, 250, " " + String(can1.getCANOutData(3), 16), menuBtnColor, menuBtnBorder, menuBtnText, LEFT);
		break;
	case 11:
		drawRoundBtn(306, 195, 347, 250, " " + String(can1.getCANOutData(4), 16), menuBtnColor, menuBtnBorder, menuBtnText, LEFT);
		break;
	case 12:
		drawRoundBtn(349, 195, 390, 250, " " + String(can1.getCANOutData(5), 16), menuBtnColor, menuBtnBorder, menuBtnText, LEFT);
		break;
	case 13:
		drawRoundBtn(392, 195, 433, 250, " " + String(can1.getCANOutData(6), 16), menuBtnColor, menuBtnBorder, menuBtnText, LEFT);
		break;
	case 14:
		drawRoundBtn(435, 195, 476, 250, " " + String(can1.getCANOutData(7), 16), menuBtnColor, menuBtnBorder, menuBtnText, LEFT);
		myGLCD.setFont(BigFont);
		break;
	case 15:
		drawRoundBtn(135, 255, 475, 300, F("Send"), menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
		break;
	case 16:
		drawSquareBtn(150, 301, 479, 319, VERSION, themeBackground, themeBackground, menuBtnColor, CENTER);
		break;
	case 17:
		return false;
		break;
	}
	return true;
}

void drawSendChannel(uint8_t channel)
{
	(channel == 0) ? drawRoundBtn(261, 55, 331, 95, F("0"), menuBtnColor, menuBtnBorder, menuBtnText, CENTER) : drawRoundBtn(261, 55, 331, 95, F("0"), menuBackground, menuBtnBorder, menuBtnText, CENTER);
	(channel == 1) ? drawRoundBtn(333, 55, 403, 95, F("1"), menuBtnColor, menuBtnBorder, menuBtnText, CENTER) : drawRoundBtn(333, 55, 403, 95, F("1"), menuBackground, menuBtnBorder, menuBtnText, CENTER);
	(channel == 2) ? drawRoundBtn(405, 55, 475, 95, F("WIFI"), menuBtnColor, menuBtnBorder, menuBtnText, CENTER) : drawRoundBtn(405, 55, 475, 95, F("WIFI"), menuBackground, menuBtnBorder, menuBtnText, CENTER);
}

// Buttons for send frame program
void sendFrameButtons(uint8_t channel)
{
	// Touch screen controls
	if (Touch_getXY())
	{
		if ((y >= 55) && (y <= 95))
		{
			if ((x >= 261) && (x <= 331))
			{
				waitForIt(261, 55, 331, 95);
				// Channel 0
				drawSendChannel((g_var8[POS0] = 0));
			}
			if ((x >= 333) && (x <= 403))
			{
				waitForIt(333, 55, 403, 95);
				// Channel 1
				drawSendChannel((g_var8[POS0] = 1));
			}
			if ((x >= 405) && (x <= 475))
			{
				waitForIt(405, 55, 475, 95);
				// WIFI
				drawSendChannel((g_var8[POS0] = 2));
			}
		}
		if ((x >= 261) && (x <= 475))
		{
			if ((y >= 100) && (y <= 145))
			{
				waitForIt(261, 100, 475, 145);
				// Set ID
				g_var8[POS2] = 0;
				resetKeypad();
				drawKeypad();
				state = 1;
				isFinished = false;
			}
		}
		if ((y >= 195) && (y <= 250))
		{
			if ((x >= 134) && (x <= 175))
			{
				waitForIt(134, 195, 175, 250);
				// Set Data[0]
				g_var8[POS2] = 0;
				g_var16[POS0] = 0;
				resetKeypad();
				drawKeypad();
				state = 2;
				isFinished = false;
			}
			if ((x >= 177) && (x <= 218))
			{
				waitForIt(177, 195, 218, 250);
				// Set Data[1]
				g_var8[POS2] = 0;
				g_var16[POS0] = 0;
				resetKeypad();
				drawKeypad();
				state = 3;
				isFinished = false;
			}
			if ((x >= 220) && (x <= 261))
			{
				waitForIt(220, 195, 261, 250);
				// Set Data[2]
				g_var8[POS2] = 0;
				g_var16[POS0] = 0;
				resetKeypad();
				drawKeypad();
				state = 4;
				isFinished = false;
			}
			if ((x >= 263) && (x <= 304))
			{
				waitForIt(263, 195, 304, 250);
				// Set Data[3]
				g_var8[POS2] = 0;
				g_var16[POS0] = 0;
				resetKeypad();
				drawKeypad();
				state = 5;
				isFinished = false;
			}
			if ((x >= 306) && (x <= 347))
			{
				waitForIt(306, 195, 347, 250);
				// Set Data[4]
				g_var8[POS2] = 0;
				g_var16[POS0] = 0;
				resetKeypad();
				drawKeypad();
				state = 6;
				isFinished = false;
			}
			if ((x >= 350) && (x <= 391))
			{
				waitForIt(349, 195, 390, 250);
				// Set Data[5]
				g_var8[POS2] = 0;
				g_var16[POS0] = 0;
				resetKeypad();
				drawKeypad();
				state = 7;
				isFinished = false;
			}
			if ((x >= 392) && (x <= 433))
			{
				waitForIt(392, 195, 433, 250);
				// Set Data[6]
				g_var8[POS2] = 0;
				g_var16[POS0] = 0;
				resetKeypad();
				drawKeypad();
				state = 8;
				isFinished = false;
			}
			if ((x >= 435) && (x <= 476))
			{
				waitForIt(435, 195, 476, 250);
				// Set Data[7]
				g_var8[POS2] = 0;
				g_var16[POS0] = 0;
				resetKeypad();
				drawKeypad();
				state = 9;
				isFinished = false;
			}
		}
		if ((x >= 135) && (x <= 475))
		{
			if ((y >= 255) && (y <= 300))
			{
				waitForIt(135, 255, 475, 300);
				// Send Frame
				can1.sendCANOut(channel, selectedChannelOut);
			}
		}
	}
}

// Using this function in sendFrame switch statement to avoid writing it out 8 times
void setData(uint8_t position)
{
	
	g_var8[POS1] = keypadController(g_var8[POS2], g_var16[POS0]);
		if (g_var8[POS1] == 0xF1) // Accept
		{
			can1.setDataCANOut(g_var16[POS0], position);
			isFinished = false;
			state = 0;
		}
		else if (g_var8[POS1] == 0xF0) // Cancel
		{
			isFinished = false;
			state = 0;
		}
	
		/*
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
	*/
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
			if (graphicLoaderState == 2)
			{
				drawSendChannel(g_var8[POS0]);
				graphicLoaderState++;
				break;
			}
			if (drawSendFrame(g_var8[POS0]))
			{
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
			sprintf(displayText, "%03X", can1.getCANOutID());
			isFinished = true;
			g_var16[POS1] = 2;
			g_var8[POS1] = 0xFF;
			g_var16[POS2] = 0;
			drawRoundBtn(255, 220, 470, 260, displayText, menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
		}

		// Keypad response
		g_var8[POS1] = keypadController(g_var8[POS2], g_var16[POS2]); // index, current input total
		if (g_var8[POS1] == 0xF1) // Accept
		{
			can1.setIDCANOut(g_var16[POS2]);
			drawRoundBtn(255, 220, 470, 260, String(can1.getCANOutID(), 16), menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
			isFinished = false;
			state = 0;
		}
		if (g_var8[POS1] == 0xF0) // Cancel
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

/*============== Timed TX ==============*/
SchedulerRX RXtimedMSG;
CAN_FRAME timedTXFRAME;
uint8_t displayedNodePosition[4];

bool drawTimedTX()
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
		drawSquareBtn(133, 56, 202, 95, F("CHL"), menuBackground, menuBtnBorder, menuBtnText, CENTER);
		break;
	case 5:
		drawSquareBtn(202, 56, 271, 95, F("ID"), menuBackground, menuBtnBorder, menuBtnText, CENTER);
		break;
	case 6:
		drawSquareBtn(271, 56, 340, 95, F("ON"), menuBackground, menuBtnBorder, menuBtnText, CENTER);
		break;
	case 7:
		drawSquareBtn(340, 56, 409, 95, F("EDIT"), menuBackground, menuBtnBorder, menuBtnText, CENTER);
		break;
	case 8:
		drawSquareBtn(409, 56, 477, 95, F("DEL"), menuBackground, menuBtnBorder, menuBtnText, CENTER);
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
		drawTXNode(g_var8[POS3]);
		break;
	case 20:
		return false;
		break;
	}
	return true;
}

bool drawTXNode(uint8_t index)
{
	g_var8[POS4] = 0;
	uint16_t yAxis = 100;
	for (uint8_t i = index; i < (index + 4); i++)
	{
		
		if (!RXtimedMSG.node[i].isDel && i < 9)
		{
			if (RXtimedMSG.node[i].channel > 1)
			{
				drawSquareBtn(133, yAxis, 202, yAxis + 40, F("WIFI"), menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
			}
			else
			{
				drawSquareBtn(133, yAxis, 202, yAxis + 40, String(RXtimedMSG.node[i].channel), menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
			}
			drawSquareBtn(202, yAxis, 271, yAxis + 40, String(RXtimedMSG.node[i].id, 16), menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
			RXtimedMSG.node[i].isOn ? drawSquareBtn(271, yAxis, 340, yAxis + 40, F("x"), menuBtnColor, menuBtnBorder, menuBtnText, CENTER) : drawSquareBtn(271, yAxis, 340, yAxis + 40, F(""), menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
			drawSquareBtn(340, yAxis, 409, yAxis + 40, F("EDIT"), menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
			drawSquareBtn(409, yAxis, 477, yAxis + 40, F("DEL"), menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
			displayedNodePosition[g_var8[POS4]] = i;
			g_var8[POS4]++;
			yAxis += 45;
		}
		else if (i < 9)
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

bool drawEditTXNode(uint8_t node)
{
	switch (graphicLoaderState)
	{
	case 0:
		break;
	case 1:
		drawSquareBtn(131, 55, 479, 319, "", themeBackground, themeBackground, themeBackground, CENTER);
		break;
	case 3:
		drawRoundBtn(135, 55, 259, 95, F("Channel"), menuBackground, menuBtnBorder, menuBtnText, CENTER);
		break;
	case 4:
		drawRoundBtn(135, 100, 200, 145, F("ID:"), menuBackground, menuBtnBorder, menuBtnText, CENTER);
		break;
	case 5:
		drawRoundBtn(202, 100, 282, 145, String(RXtimedMSG.node[node].id, 16), menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
		drawRoundBtn(284, 100, 354, 145, F("INT:"), menuBackground, menuBtnBorder, menuBtnText, CENTER);
		drawRoundBtn(356, 100, 475, 145, String(RXtimedMSG.node[node].interval), menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
		break;
	case 6:
		drawRoundBtn(135, 150, 475, 190, F("FRAME"), menuBackground, menuBtnBorder, menuBtnText, CENTER);
		break;
	case 7:
		myGLCD.setFont(SmallFont);
		drawRoundBtn(134, 195, 175, 250, " " + String(RXtimedMSG.node[g_var8[POS0]].data[0], 16), menuBtnColor, menuBtnBorder, menuBtnText, LEFT);
		break;
	case 8:
		drawRoundBtn(177, 195, 218, 250, " " + String(RXtimedMSG.node[g_var8[POS0]].data[1], 16), menuBtnColor, menuBtnBorder, menuBtnText, LEFT);
		break;
	case 9:
		drawRoundBtn(220, 195, 261, 250, " " + String(RXtimedMSG.node[g_var8[POS0]].data[2], 16), menuBtnColor, menuBtnBorder, menuBtnText, LEFT);
		break;
	case 10:
		drawRoundBtn(263, 195, 304, 250, " " + String(RXtimedMSG.node[g_var8[POS0]].data[3], 16), menuBtnColor, menuBtnBorder, menuBtnText, LEFT);
		break;
	case 11:
		drawRoundBtn(306, 195, 347, 250, " " + String(RXtimedMSG.node[g_var8[POS0]].data[4], 16), menuBtnColor, menuBtnBorder, menuBtnText, LEFT);
		break;
	case 12:
		drawRoundBtn(349, 195, 390, 250, " " + String(RXtimedMSG.node[g_var8[POS0]].data[5], 16), menuBtnColor, menuBtnBorder, menuBtnText, LEFT);
		break;
	case 13:
		drawRoundBtn(392, 195, 433, 250, " " + String(RXtimedMSG.node[g_var8[POS0]].data[6], 16), menuBtnColor, menuBtnBorder, menuBtnText, LEFT);
		break;
	case 14:
		drawRoundBtn(435, 195, 476, 250, " " + String(RXtimedMSG.node[g_var8[POS0]].data[7], 16), menuBtnColor, menuBtnBorder, menuBtnText, LEFT);
		myGLCD.setFont(BigFont);
		break;
	case 15:
		drawRoundBtn(135, 255, 304, 300, F("Accept"), menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
		drawRoundBtn(306, 255, 475, 300, F("Cancel"), menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
		break;
	case 16:
		drawSquareBtn(150, 301, 479, 319, VERSION, themeBackground, themeBackground, menuBtnColor, CENTER);
		break;
	case 17:
		drawSendChannel(RXtimedMSG.node[g_var8[POS0]].channel);
		return false;
		break;
	}
	graphicLoaderState++;
	return true;
}

void timedTXButtons()
{
	if (Touch_getXY())
	{
		if ((y >= 100) && (y <= 140) && g_var8[POS4] > 0)
		{
			if ((x >= 271) && (x <= 340))
			{
				waitForIt(271, 100, 340, 140);
				// On
				RXtimedMSG.node[displayedNodePosition[0]].isOn = !RXtimedMSG.node[displayedNodePosition[0]].isOn;
				RXtimedMSG.node[displayedNodePosition[0]].isOn ? drawSquareBtn(271, 100, 340, 140, F("x"), menuBtnColor, menuBtnBorder, menuBtnText, CENTER) : drawSquareBtn(271, 100, 340, 140, F(""), menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
			}
			if ((x >= 340) && (x <= 409))
			{
				waitForIt(340, 100, 409, 140);
				// Edit
				g_var8[POS0] = displayedNodePosition[0];
				state = 2;
			}
			if ((x >= 409) && (x <= 477))
			{
				waitForIt(409, 100, 477, 140);
				// Del
				RXtimedMSG.node[displayedNodePosition[0]].isDel = true;
				RXtimedMSG.node[displayedNodePosition[0]].isOn = false;
				RXtimedMSG.node[displayedNodePosition[0]].id = 0;
				RXtimedMSG.node[displayedNodePosition[0]].interval = 0;
				RXtimedMSG.node[displayedNodePosition[0]].channel = 0;
				for (uint8_t i = 0; i < 8; i++)
				{
					RXtimedMSG.node[displayedNodePosition[0]].data[i] = 0;
				}
				drawTXNode(g_var8[POS3]);
			}
		}
		if ((y >= 145) && (y <= 185) && g_var8[POS4] > 1)
		{
			if ((x >= 271) && (x <= 340))
			{
				waitForIt(271, 145, 340, 185);
				// On
				RXtimedMSG.node[displayedNodePosition[1]].isOn = !RXtimedMSG.node[displayedNodePosition[1]].isOn;
				RXtimedMSG.node[displayedNodePosition[1]].isOn ? drawSquareBtn(271, 145, 340, 185, F("x"), menuBtnColor, menuBtnBorder, menuBtnText, CENTER) : drawSquareBtn(271, 145, 340, 185, F(""), menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
			}
			if ((x >= 340) && (x <= 409))
			{
				waitForIt(340, 145, 409, 185);
				// Edit
				g_var8[POS0] = displayedNodePosition[1];
				state = 2;
			}
			if ((x >= 409) && (x <= 477))
			{
				waitForIt(409, 145, 477, 185);
				// Del
				RXtimedMSG.node[displayedNodePosition[1]].isDel = true;
				RXtimedMSG.node[displayedNodePosition[1]].isOn = false;
				RXtimedMSG.node[displayedNodePosition[1]].id = 0;
				RXtimedMSG.node[displayedNodePosition[1]].interval = 0;
				RXtimedMSG.node[displayedNodePosition[1]].channel = 0;
				for (uint8_t i = 0; i < 8; i++)
				{
					RXtimedMSG.node[displayedNodePosition[1]].data[i] = 0;
				}
				drawTXNode(g_var8[POS3]);
			}
		}
		if ((y >= 190) && (y <= 230) && g_var8[POS4] > 2)
		{
			if ((x >= 271) && (x <= 340))
			{
				waitForIt(271, 190, 340, 230);
				// On
				RXtimedMSG.node[displayedNodePosition[2]].isOn = !RXtimedMSG.node[displayedNodePosition[2]].isOn;
				RXtimedMSG.node[displayedNodePosition[2]].isOn ? drawSquareBtn(271, 190, 340, 230, F("x"), menuBtnColor, menuBtnBorder, menuBtnText, CENTER) : drawSquareBtn(271, 190, 340, 230, F(""), menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
			}
			if ((x >= 340) && (x <= 409))
			{
				waitForIt(340, 190, 409, 230);
				// Edit
				g_var8[POS0] = displayedNodePosition[2];
				state = 2;
			}
			if ((x >= 409) && (x <= 477))
			{
				waitForIt(409, 190, 477, 230);
				// Del
				RXtimedMSG.node[displayedNodePosition[2]].isDel = true;
				RXtimedMSG.node[displayedNodePosition[2]].isOn = false;
				RXtimedMSG.node[displayedNodePosition[2]].id = 0;
				RXtimedMSG.node[displayedNodePosition[2]].interval = 0;
				RXtimedMSG.node[displayedNodePosition[2]].channel = 0;
				for (uint8_t i = 0; i < 8; i++)
				{
					RXtimedMSG.node[displayedNodePosition[2]].data[i] = 0;
				}
				drawTXNode(g_var8[POS3]);
			}
		}
		if ((y >= 235) && (y <= 275) && g_var8[POS4] > 3)
		{
			if ((x >= 271) && (x <= 340))
			{
				waitForIt(271, 235, 340, 275);
				// On
				RXtimedMSG.node[displayedNodePosition[3]].isOn = !RXtimedMSG.node[displayedNodePosition[3]].isOn;
				RXtimedMSG.node[displayedNodePosition[3]].isOn ? drawSquareBtn(271, 235, 340, 275, F("x"), menuBtnColor, menuBtnBorder, menuBtnText, CENTER) : drawSquareBtn(271, 235, 340, 275, F(""), menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
			}
			if ((x >= 340) && (x <= 409))
			{
				waitForIt(340, 235, 409, 275);
				// Edit
				g_var8[POS0] = displayedNodePosition[3];
				state = 2;
			}
			if ((x >= 409) && (x <= 477))
			{
				waitForIt(409, 235, 477, 275);
				// Del
				RXtimedMSG.node[displayedNodePosition[3]].isDel = true;
				RXtimedMSG.node[displayedNodePosition[3]].isOn = false;
				RXtimedMSG.node[displayedNodePosition[3]].id = 0;
				RXtimedMSG.node[displayedNodePosition[3]].interval = 0;
				RXtimedMSG.node[displayedNodePosition[3]].channel = 0;
				for (uint8_t i = 0; i < 8; i++)
				{
					RXtimedMSG.node[displayedNodePosition[3]].data[i] = 0;
				}
				drawTXNode(g_var8[POS3]);
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
					drawTXNode(g_var8[POS3]);
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
				if (g_var8[POS3] < 5)
				{
					g_var8[POS3]++;
					drawTXNode(g_var8[POS3]);
				}
			}
		}
	}
}

void timedTX()
{
	switch (state)
	{
	case 0:
		if (drawTimedTX())
		{
			graphicLoaderState++;
		}
		timedTXButtons();
		break;
	case 1:
		// Add state
		g_var8[POS0] = findFreeTXNode();
		// 0xFF = no free slots
		(g_var8[POS0] == 0xFF) ? state = 0 : state = 2;
		break;
	case 2:
		drawEditTXNode(g_var8[POS0]) ? state = 2 : state = 3;
		break;
	case 3:
		editTXNodeButtons();
		break;
	case 4:
		//ID
		g_var8[POS1] = keypadController(g_var8[POS2], g_var16[POS0]);
		if (g_var8[POS1] == 0xF1) // Accept
		{
			graphicLoaderState = 0;
			RXtimedMSG.node[g_var8[POS0]].id = g_var16[POS0];
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
			RXtimedMSG.node[g_var8[POS0]].interval = g_var16[POS0];
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
			RXtimedMSG.node[g_var8[POS0]].data[0] = g_var16[POS0];
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
			RXtimedMSG.node[g_var8[POS0]].data[1] = g_var16[POS0];
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
			RXtimedMSG.node[g_var8[POS0]].data[2] = g_var16[POS0];
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
			RXtimedMSG.node[g_var8[POS0]].data[3] = g_var16[POS0];
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
			RXtimedMSG.node[g_var8[POS0]].data[4] = g_var16[POS0];
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
			RXtimedMSG.node[g_var8[POS0]].data[5] = g_var16[POS0];
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
			RXtimedMSG.node[g_var8[POS0]].data[6] = g_var16[POS0];
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
			RXtimedMSG.node[g_var8[POS0]].data[7] = g_var16[POS0];
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

void editTXNodeButtons()
{
	// Touch screen controls
	if (Touch_getXY())
	{
		if ((y >= 55) && (y <= 95))
		{
			if ((x >= 261) && (x <= 331))
			{
				waitForIt(261, 55, 331, 95);
				// Channel 0
				RXtimedMSG.node[g_var8[POS0]].channel = 0;
				drawSendChannel(RXtimedMSG.node[g_var8[POS0]].channel);
			}
			if ((x >= 333) && (x <= 403))
			{
				waitForIt(333, 55, 403, 95);
				// Channel 1
				RXtimedMSG.node[g_var8[POS0]].channel = 1;
				drawSendChannel(RXtimedMSG.node[g_var8[POS0]].channel);
			}
			if ((x >= 405) && (x <= 475))
			{
				waitForIt(405, 55, 475, 95);
				// WIFI
				RXtimedMSG.node[g_var8[POS0]].channel = 2;
				drawSendChannel(RXtimedMSG.node[g_var8[POS0]].channel);
			}
		}
		if ((y >= 100) && (y <= 145))
		{
			if ((x >= 202) && (x <= 282))
			{
				waitForIt(202, 100, 282, 145);
				// Set ID
				g_var8[POS2] = 0;
				g_var16[POS0] = 0;
				resetKeypad();
				drawKeypad();
				state = 4;
			}
			if ((x >= 356) && (x <= 475))
			{
				waitForIt(356, 100, 475, 145);
				// Set Interval
				g_var8[POS2] = 0;
				g_var16[POS0] = 0;
				resetKeypad();
				drawKeypadDec();
				state = 5;
			}
		}
		if ((y >= 195) && (y <= 250))
		{
			if ((x >= 134) && (x <= 175))
			{
				waitForIt(134, 195, 175, 250);
				// Set Data[0]
				g_var8[POS2] = 0;
				g_var16[POS0] = 0;
				resetKeypad();
				state = 6;
				drawKeypad();
			}
			if ((x >= 177) && (x <= 218))
			{
				waitForIt(177, 195, 218, 250);
				// Set Data[1]
				g_var8[POS2] = 0;
				g_var16[POS0] = 0;
				resetKeypad();
				state = 7;
				drawKeypad();
			}
			if ((x >= 220) && (x <= 261))
			{
				waitForIt(220, 195, 261, 250);
				// Set Data[2]
				g_var8[POS2] = 0;
				g_var16[POS0] = 0;
				resetKeypad();
				state = 8;
				drawKeypad();
			}
			if ((x >= 263) && (x <= 304))
			{
				waitForIt(263, 195, 304, 250);
				// Set Data[3]
				g_var8[POS2] = 0;
				g_var16[POS0] = 0;
				resetKeypad();
				state = 9;
				drawKeypad();
			}
			if ((x >= 306) && (x <= 347))
			{
				waitForIt(306, 195, 347, 250);
				// Set Data[4]
				g_var8[POS2] = 0;
				g_var16[POS0] = 0;
				resetKeypad();
				state = 10;
				drawKeypad();
			}
			if ((x >= 350) && (x <= 391))
			{
				waitForIt(349, 195, 390, 250);
				g_var8[POS2] = 0;
				g_var16[POS0] = 0;
				resetKeypad();
				// Set Data[5]
				state = 11;
				drawKeypad();
			}
			if ((x >= 392) && (x <= 433))
			{
				waitForIt(392, 195, 433, 250);
				// Set Data[6]
				g_var8[POS2] = 0;
				g_var16[POS0] = 0;
				resetKeypad();
				state = 12;
				drawKeypad();
			}
			if ((x >= 435) && (x <= 476))
			{
				waitForIt(435, 195, 476, 250);
				// Set Data[7]
				g_var8[POS2] = 0;
				g_var16[POS0] = 0;
				resetKeypad();
				state = 13;
				drawKeypad();
			}
		}
		if ((y >= 255) && (y <= 300))
		{
			if ((x >= 135) && (x <= 304))
			{
				waitForIt(135, 255, 304, 300);
				// Accept
				graphicLoaderState = 0;
				RXtimedMSG.node[g_var8[POS0]].isDel = false;
				state = 0;
			}
			if ((x >= 306) && (x <= 475))
			{
				waitForIt(306, 255, 475, 300);
				// Cancel
				graphicLoaderState = 0;
				state = 0;
			}
		}
	}
}

uint8_t findFreeTXNode()
{
	for (uint8_t i = 0; i < TIMED_TX_MAX_SIZE; i++)
	{
		if (RXtimedMSG.node[i].isDel)
		{
			return i;
		}
	}
	return 0xFF;
}

void timedTXSend()
{
	for (uint8_t i = 0; i < 10; i++)
	{
		if (!RXtimedMSG.node[i].isDel && RXtimedMSG.node[i].isOn && (millis() - RXtimedMSG.node[i].timer > RXtimedMSG.node[i].interval))
		{
			timedTXFRAME.id = RXtimedMSG.node[i].id;
			timedTXFRAME.length = 8;
			timedTXFRAME.extended = false;
			for (uint8_t j = 0; j < 8; j++)
			{
				timedTXFRAME.data.bytes[i] = RXtimedMSG.node[i].data[j];
			}
			can1.sendCANOut(RXtimedMSG.node[i].channel, timedTXFRAME, false);
			RXtimedMSG.node[i].timer = millis();
		}
	}
}

/*============== Baud ==============*/
// Draw the stationary baud page buttons
bool drawBaud()
{
	switch (graphicLoaderState)
	{
	case 0:
		break;
	case 1:
		drawSquareBtn(131, 55, 479, 319, "", themeBackground, themeBackground, themeBackground, CENTER);
		break;
	case 2:
		drawSquareBtn(140, 275, 300, 315, F("Set CAN0"), menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
		break;
	case 3:
		drawSquareBtn(305, 275, 475, 315, F("Set CAN1"), menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
		break;
	case 4:
		drawSquareBtn(305, 60, 475, 100, F("CAN0"), menuBackground, menuBtnBorder, menuBtnText, CENTER);
		break;
	case 5:
		drawSquareBtn(305, 150, 475, 190, F("CAN1"), menuBackground, menuBtnBorder, menuBtnText, CENTER);
		break;
	case 6:
		return false;
		break;
	}
	return true;
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
	if (Touch_getXY())
	{
		if ((x >= 140) && (x <= 300))
		{
			if ((y >= 60) && (y <= 95))
			{
				waitForItRect(140, 60, 300, 95);
				g_var32[POS0] = baudRates[0];
			}
			if ((y >= 95) && (y <= 130))
			{
				waitForItRect(140, 95, 300, 130);
				g_var32[POS0] = baudRates[1];
			}
			if ((y >= 130) && (y <= 165))
			{
				waitForItRect(140, 130, 300, 165);
				g_var32[POS0] = baudRates[2];
			}
			if ((y >= 165) && (y <= 200))
			{
				waitForItRect(140, 165, 300, 200);
				g_var32[POS0] = baudRates[3];
			}
			if ((y >= 200) && (y <= 235))
			{
				waitForItRect(140, 200, 300, 235);
				g_var32[POS0] = baudRates[4];
			}
			if ((y >= 235) && (y <= 270))
			{
				waitForItRect(140, 235, 300, 270);
				g_var32[POS0] = baudRates[5];
			}
		}
		if ((y >= 275) && (y <= 315))
		{
			if ((x >= 140) && (x <= 300))
			{
				// CAN0
				waitForItRect(140, 275, 300, 315);
				can1.setBaud0(g_var32[POS0]);
				drawCurrentBaud();
			}
			if ((x >= 305) && (x <= 465))
			{
				// CAN21
				waitForItRect(305, 275, 475, 315);
				can1.setBaud1(g_var32[POS0]);
				drawCurrentBaud();
			}
		}
	}
}

// Auto baud
void findBaud()
{
	can1.setBaud0(can1.findBaudRate0());
	can1.setBaud1(can1.findBaudRate1());
	openAllTraffic();
}

/*============== Filter Mask ==============*/
// Draw the filter mask page
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

// Buttons for the filter mask program
void filterMaskButtons()
{
	// Touch screen controls
	if (Touch_getXY())
	{
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

// Open up filter mask to receive all traffic
void openAllTraffic()
{
	CAN0Filter = 000;
	CAN0Mask = 0x7FF;
	CAN1Filter = 0x000;
	CAN1Mask = 0x7FF;
	can1.startCAN0(CAN0Filter, CAN0Mask);
	can1.startCAN1(CAN1Filter, CAN1Mask);
}

// Filter mask program
uint8_t setFilterMask(uint32_t& value)
{
	char displayText[10];
	if (!isFinished)
	{
		drawKeypad();
		isFinished = true;
		g_var16[POS0] = 2;
		g_var32[POS0] = 0xFF;
		g_var32[POS1] = 0;
		drawRoundBtn(145, 220, 470, 260, F("000"), menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
	}

	g_var32[POS0] = keypadButtons();
	if (g_var32[POS0] >= 0x00 && g_var32[POS0] < 0x10 && g_var16[POS0] >= 0)
	{
		// CAN0Filter = current value + returned keypad value times its hex place
		g_var32[POS1] = g_var32[POS1] + (g_var32[POS0] * hexTable[g_var16[POS0]]);
		sprintf(displayText, "%03X", g_var32[POS1]);
		drawRoundBtn(145, 220, 470, 260, displayText, menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
		if (g_var16[POS0] >= 0)
		{
			g_var16[POS0]--;
		}
		g_var32[POS0] = 0xFF;
	}
	if (g_var32[POS0] == 0x10)
	{
		// Clear
		g_var16[POS0] = 2;
		g_var32[POS0] = 0;
		g_var32[POS1] = 0;
		sprintf(displayText, "%03X", g_var32[POS1]);
		drawRoundBtn(145, 220, 470, 260, displayText, menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
	}
	if (g_var32[POS0] == 0x11)
	{
		// Accept
		isFinished = false;
		graphicLoaderState = 0;
		state = 0;
		value = g_var32[POS1];
		return 0x11;
	}
	if (g_var32[POS0] == 0x12)
	{
		// Cancel
		isFinished = false;
		graphicLoaderState = 0;
		state = 0;
	}
	return 0;
}

// Set new filter mask
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
// Draw the capture playback page
bool drawCANLog()
{
	switch (graphicLoaderState)
	{
	case 0:

		break;
	case 1:
		drawSquareBtn(131, 55, 479, 319, "", themeBackground, themeBackground, themeBackground, CENTER);
		break;
	case 2:
		drawSquareBtn(420, 80, 470, 160, F("/\\"), menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
		break;
	case 3:
		drawSquareBtn(420, 160, 470, 240, F("\\/"), menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
		break;
	case 4:
		drawSquareBtn(131, 275, 216, 315, F("Play"), menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
		break;
	case 5:
		drawSquareBtn(216, 275, 301, 315, F("Del"), menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
		break;
	case 6:
		drawSquareBtn(301, 275, 386, 315, F("Split"), menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
		break;
	case 7:
		drawSquareBtn(386, 275, 479, 315, F("Conf"), menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
		break;
	case 8:
		return false;
		break;
	}
	return true;
}

// Draw playable filelist
void drawCANLogScroll()
{
	File root1;
	root1 = SD.open("/");

	uint8_t size = sdCard.printDirectory(root1, fileList);

	// Starting y location for list
	uint16_t y = 60;

	//drawSquareBtn(131, 80, 415, 240, "", themeBackground, themeBackground, themeBackground, CENTER);

	// Draw the scroll window
	for (uint8_t i = 0; i < MAXSCROLL; i++)
	{
		if ((g_var8[POS0] + i < 10))
		{
			char temp[13];
			sprintf(temp, "%s", fileList[(g_var8[POS0] + i)]);
			drawSquareBtn(150, y, 410, y + 35, temp, menuBackground, menuBtnBorder, menuBtnText, LEFT);
		}
		else
		{
			drawSquareBtn(150, y, 410, y + 35, "", menuBackground, menuBtnBorder, menuBtnText, LEFT);
		}
		y = y + 35;
	}
}

// Buttons for the capture playback program
void CANLogButtons()
{
	// Touch screen controls
	if (Touch_getXY())
	{
		if ((x >= 150) && (x <= 410))
		{
			if ((y >= 60) && (y <= 95))
			{
				waitForItRect(150, 60, 410, 95);
				g_var16[POS0] = 0 + g_var8[POS0];
			}
			if ((y >= 95) && (y <= 130))
			{
				waitForItRect(150, 95, 410, 130);
				g_var16[POS0] = 1 + g_var8[POS0];
			}
			if ((y >= 130) && (y <= 165))
			{
				waitForItRect(150, 130, 410, 165);
				g_var16[POS0] = 2 + g_var8[POS0];
			}
			if ((y >= 165) && (y <= 200))
			{
				waitForItRect(150, 165, 410, 200);
				g_var16[POS0] = 3 + g_var8[POS0];
			}
			if ((y >= 200) && (y <= 235))
			{
				waitForItRect(150, 200, 410, 235);
				g_var16[POS0] = 4 + g_var8[POS0];
			}
			if ((y >= 235) && (y <= 270))
			{
				waitForItRect(150, 235, 410, 270);
				g_var16[POS0] = 5 + g_var8[POS0];
			}
		}
		if ((x >= 420) && (x <= 470))
		{
			if ((y >= 80) && (y <= 160))
			{
				waitForItRect(420, 80, 470, 160);
				if (g_var8[POS0] > 0)
				{
					g_var8[POS0]--;
					drawCANLogScroll();
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
					g_var8[POS0]++;
					drawCANLogScroll();
				}
			}
		}
		if ((y >= 275) && (y <= 315))
		{
			char fileLoc[20] = "CANLOG/";
			strcat(fileLoc, fileList[g_var16[POS0]]);
			if ((x >= 131) && (x <= 216))
			{
				// Play
				waitForItRect(131, 275, 216, 315);
				drawSquareBtn(131, 275, 216, 315, F("Stop"), menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
				delay(200);
				sdCard.readLogFile(fileLoc);
				delay(200);
				myTouch.read();
				drawSquareBtn(131, 275, 216, 315, F("Start"), menuBtnColor, menuBtnBorder, menuBtnText, CENTER);

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