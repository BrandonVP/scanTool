/*
 ===========================================================================
 Name        : CANBusCapture.cpp
 Author      : Brandon Van Pelt
 Created	 : 11/15/2020 8:54:18 AM
 Description : CAN Bus capture and monitor tools
 ===========================================================================
 */

#define _CANBUSCAPTURE_C
#include "CANBusCapture.h"
#include "common.h"

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
		drawRoundBtn(310, 80, 475, 130, F("Send"), menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
		break;
	case 4:
		drawRoundBtn(140, 135, 305, 185, F("Cap Files"), menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
		break;
	case 5:
		drawRoundBtn(310, 135, 475, 185, F("Baud"), menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
		break;
	case 6:
		drawRoundBtn(140, 190, 305, 240, F("FilterMask"), menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
		break;
	case 7:
		drawRoundBtn(310, 190, 475, 240, F("Auto Baud"), menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
		break;
	case 8:
		drawRoundBtn(140, 245, 305, 295, F(""), menuBackground, menuBtnBorder, menuBtnText, CENTER);
		break;
	case 9:
		drawRoundBtn(310, 245, 475, 295, F(""), menuBackground, menuBtnBorder, menuBtnText, CENTER);
		break;
	case 10:
		drawSquareBtn(150, 301, 479, 319, VERSION, themeBackground, themeBackground, menuBtnColor, CENTER);
		break;
	case 11:
		return false;
		break;
	}
	graphicLoaderState++;
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
				state = 0;
				nextPage = 1;
				graphicLoaderState = 0;
			}
			if ((y >= 135) && (y <= 185))
			{
				waitForIt(140, 135, 305, 185);
				// Playback
				graphicLoaderState = 0;
				nextPage = 2;
				
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
				waitForIt(140, 245, 305, 295);
				// Unused
			}
		}
		if ((x >= 310) && (x <= 475))
		{
			if ((y >= 80) && (y <= 130))
			{
				waitForIt(310, 80, 475, 130);
				// Timed TX
				graphicLoaderState = 0;
				nextPage = 6;
			}
			if ((y >= 135) && (y <= 185))
			{
				waitForIt(310, 135, 475, 185);
				// Baud
				graphicLoaderState = 0;
				nextPage = 5;
			}
			if ((y >= 190) && (y <= 240))
			{
				waitForIt(310, 190, 475, 240);
				// Find Baud
				findBaud();
			}
			if ((y >= 245) && (y <= 295))
			{
				waitForIt(310, 245, 475, 295);
				// Unused
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
bool drawCaptureSelected()
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
	case 7: 
		return false;
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
	case 4:
		return false;
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
		drawSquareBtn(135, 85, 300, 125, F("LCD"), menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
		//selectedChannelOut == 6 ? drawSquareBtn(135, 85, 300, 125, F("LCD"), menuBackground, menuBtnBorder, menuBtnText, CENTER) : drawSquareBtn(135, 85, 300, 125, F("LCD"), menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
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
				else if (state == 2)
				//else if (state == 2 && selectedChannelOut != 6) // LCD wont work with WIFI
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

				// Clear the WiFi CAN Bus buffer
				if (selectedChannelOut == 6)
				{
					uint8_t value = 0;
					for (uint16_t i = 0; i < 1000; i++)
					{
						if (Serial3.available())
						{
							value = Serial3.read();
						}
					}
				}
				else
				{
					Can0.empty_rx_buff();
					Can1.empty_rx_buff();
				}

				can1.resetMessageNum();
				switch (selectedSourceOut)
				{
				case 1: // LCD
					nextPage = 7;
					g_var16[POS0] = 60;
					break;
				case 2: // Serial
					
					isSerialOut = true;
					drawSquareBtn(310, 185, 470, 240, F("Start"), menuBackground, menuBtnBorder, menuBtnText, CENTER);
					drawSquareBtn(310, 245, 470, 300, F("Stop"), menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
					break;
				case 3: // SD Card
					// TODO: Put this in a state and remove blocking code
					bool setName = true;
					uint8_t result = 0;
					uint8_t index = 0;
					char filename[12];
					//char* a1 = filename;

					drawkeyboard();
					while (setName)
					{
						// Name
						result = keyboardController(index);
						if (result == 0xF1) // Accept
						{
							//strcat(filename, keyboardInput);
							strncpy(filename, keyboardInput, 9);
							//a1 += index;
							strcat(filename, ".txt");
							//strncpy(a1, ".txt", 4);
							for (uint8_t i = 0; i < 9; i++)
							{
								keyboardInput[i] = '\0';
							}
							setName = false;
						}
						else if (result == 0xF0) // Cancel
						{
							setName = false;
						}
						backgroundProcess();
					}
					sdCard.setSDFilename(filename);
					for (uint8_t i = 0; i < 12; i++)
					{
						filename[i] = '\0';
					}
					isSDOut = true;
					hasDrawn = false;
					graphicLoaderState = 1;
					drawSquareBtn(131, 55, 479, 319, "", themeBackground, themeBackground, themeBackground, CENTER);
					drawSquareBtn(310, 185, 470, 240, F("Start"), menuBackground, menuBtnBorder, menuBtnText, CENTER);
					drawSquareBtn(310, 245, 470, 300, F("Stop"), menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
					break;
				}
			}
			if ((y >= 245) && (y <= 285))
			{
				waitForIt(310, 245, 470, 300);
				// Stop
				if (isSDOut)
				{
					// Save messages in SD buffer
					char empty[2];
					SDCardBuffer(empty, true);
				}
					
				isSerialOut = false;
				isSDOut = false;
				drawSquareBtn(310, 185, 470, 240, F("Start"), menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
				
			}
		}
	}
}

// Clears page before starting capture
void drawReadInCANLCD()
{
	drawSquareBtn(131, 55, 479, 319, "", themeBackground, themeBackground, themeBackground, CENTER);
}

// Convert hex value (uint8_t value 0x1B --> char result[2] = {'1', 'B,}
void hex2String(uint8_t value, char(&result)[N])
{
	//uint8_t hNibble = ((value >> 4) & 0x0f);
	//uint8_t lNibble = (value & 0x0f);

	if (((value >> 4) & 0x0f) < 10)
	{
		result[0] = (((value >> 4) & 0x0f) + 48);
	}
	else
	{
		result[0] = (((value >> 4) & 0x0f) + 55);
	}
	if ((value & 0x0f) < 10)
	{
		result[1] = (value & 0x0f) + 48;
	}
	else
	{
		result[1] = (value & 0x0f) + 55;
	}
}

// Capture to LCD, max rate without filling buffer is 12ms per message (slow), useful for bench testings or filtered traffic
void readInCANMsg(uint8_t channel)
{
	//uint32_t test = millis();
	
	uint8_t rxBuf[8];
	uint32_t rxId;
	uint8_t len;
	if (can1.LCDOutCAN(rxBuf, len, rxId, channel))
	{
		myGLCD.setFont(SmallFont);
		myGLCD.setBackColor(VGA_WHITE);

		if (g_var16[POS0] != 60)
		{
			myGLCD.setColor(VGA_WHITE);
			myGLCD.fillRect(140, (g_var16[POS0] - 15), 144, (g_var16[POS0] - 5));
			myGLCD.setColor(VGA_BLACK);
			myGLCD.fillRect(140, (g_var16[POS0]), 144, (g_var16[POS0] + 10));
		}
		else
		{
			myGLCD.setColor(VGA_WHITE);
			myGLCD.fillRect(140, 300, 144, 310);
			myGLCD.setColor(VGA_BLACK);
			myGLCD.fillRect(140, (g_var16[POS0]), 144, (g_var16[POS0] + 10));
		}

		// LCD slows down printing this large char
		// Rolling it out below reduced function call by 11ms!
		// TODO: Writing a function to convert the hex value to char might decrease function call time
		//char printString[40];
		//sprintf(printString, "%03X  %d  %02X  %02X  %02X  %02X  %02X  %02X  %02X  %02X", rxId, len, rxBuf[0], rxBuf[1], rxBuf[2], rxBuf[3], rxBuf[4], rxBuf[5], rxBuf[6], rxBuf[7]);
		//myGLCD.print(printString, 151, g_var16[POS0]);

		char temp1[3];
		char temp2[2];

		sprintf(temp1, "%03X", rxId);
		myGLCD.print(temp1, 151, g_var16[POS0]);

		myGLCD.printNumI(len, 196, g_var16[POS0]);

		//hex2String(rxBuf[0], temp2);
		sprintf(temp2, "%02X", rxBuf[0]);
		myGLCD.print(temp2, 226, g_var16[POS0]);

		//hex2String(rxBuf[1], temp2);
		sprintf(temp2, "%02X", rxBuf[1]);
		myGLCD.print(temp2, 256, g_var16[POS0]);

		//hex2String(rxBuf[2], temp2);
		sprintf(temp2, "%02X", rxBuf[2]);
		myGLCD.print(temp2, 286, g_var16[POS0]);

		hex2String(rxBuf[3], temp2);
		sprintf(temp2, "%02X", rxBuf[3]);
		myGLCD.print(temp2, 316, g_var16[POS0]);

		//hex2String(rxBuf[4], temp2);
		sprintf(temp2, "%02X", rxBuf[4]);
		myGLCD.print(temp2, 346, g_var16[POS0]);

		//hex2String(rxBuf[5], temp2);
		sprintf(temp2, "%02X", rxBuf[5]);
		myGLCD.print(temp2, 376, g_var16[POS0]);

		//hex2String(rxBuf[6], temp2);
		sprintf(temp2, "%02X", rxBuf[6]);
		myGLCD.print(temp2, 406, g_var16[POS0]);

		//hex2String(rxBuf[7], temp2);
		sprintf(temp2, "%02X", rxBuf[7]);
		myGLCD.print(temp2, 436, g_var16[POS0]);

		if (g_var16[POS0] < 300)
		{
			g_var16[POS0] += 15;
		}
		else
		{
			g_var16[POS0] = 60;
		}
		myGLCD.setFont(BigFont);

		//uint32_t temp = millis() - test;
		//SerialUSB.print("Time: ");
		//SerialUSB.println(temp);
	}
}

/*============== Timed TX ==============*/


void drawSendChannel(uint8_t channel)
{
	(channel == 0) ? drawRoundBtn(261, 99, 331, 139, F("0"), menuBtnColor, menuBtnBorder, menuBtnText, CENTER) : drawRoundBtn(261, 99, 331, 139, F("0"), menuBackground, menuBtnBorder, menuBtnText, CENTER);
	(channel == 1) ? drawRoundBtn(333, 99, 403, 139, F("1"), menuBtnColor, menuBtnBorder, menuBtnText, CENTER) : drawRoundBtn(333, 99, 403, 139, F("1"), menuBackground, menuBtnBorder, menuBtnText, CENTER);
	(channel == 2) ? drawRoundBtn(405, 99, 475, 139, F("WIFI"), menuBtnColor, menuBtnBorder, menuBtnText, CENTER) : drawRoundBtn(405, 99, 475, 139, F("WIFI"), menuBackground, menuBtnBorder, menuBtnText, CENTER);
}

void loadRXMsg()
{
	sdCard.readSendMsg(RXtimedMSG);
}

void saveRXMsg()
{
	sdCard.writeSendMsg(RXtimedMSG);
}

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
		drawSquareBtn(132, 55, 478, 96, F(""), menuBackground, menuBtnBorder, menuBtnText, CENTER);
		break;
	case 5:
		drawSquareBtn(132, 99, 478, 141, F(""), menuBackground, menuBtnBorder, menuBtnText, CENTER);
		break;
	case 6:
		drawSquareBtn(132, 144, 478, 186, F(""), menuBackground, menuBtnBorder, menuBtnText, CENTER);
		break;
	case 7:
		drawSquareBtn(132, 189, 478, 231, F(""), menuBackground, menuBtnBorder, menuBtnText, CENTER);
		break;
	case 8:
		drawSquareBtn(132, 279, 211, 318, F(""), menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
		break;	
	case 9:
		drawSquareBtn(132, 234, 478, 276, F(""), menuBackground, menuBtnBorder, menuBtnText, CENTER);
		break;
	case 10:
		drawSquareBtn(133, 280, 210, 317, F("/\\"), menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
		break;
	case 11:
		drawSquareBtn(260, 279, 351, 318, F(""), menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
		break;
	case 12:
		drawSquareBtn(261, 280, 350, 317, F("Add"), menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
		break;
	case 13:
		drawSquareBtn(400, 279, 478, 318, F(""), menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
		break;
	case 14:
		drawSquareBtn(401, 280, 477, 317, F("\\/"), menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
		break;
	case 15:
		if (hasDrawn == true)
		{
			drawTXNode(g_var8[POS3]);
		}
		break;
	case 16:
		return false;
		break;
	}

	graphicLoaderState++;
	return true;
}

bool drawTXNode(uint8_t index)
{
	g_var8[POS4] = 0;
	uint16_t yAxis = 55;
	for (uint8_t i = index; i < (index + 5); i++)
	{
		if (!RXtimedMSG.node[i].isDel && i < 20)
		{
			drawSquareBtn(133, yAxis, 271, yAxis + 40, RXtimedMSG.node[i].name, menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
			RXtimedMSG.node[i].isOn ? drawSquareBtn(271, yAxis, 340, yAxis + 40, F("ON"), menuBtnColor, menuBtnBorder, menuBtnText, CENTER) : drawSquareBtn(271, yAxis, 340, yAxis + 40, F("OFF"), menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
			drawSquareBtn(340, yAxis, 409, yAxis + 40, F("EDIT"), menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
			drawSquareBtn(409, yAxis, 477, yAxis + 40, F("DEL"), menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
			displayedNodePosition[g_var8[POS4]] = i;
			g_var8[POS4]++;
			yAxis += 45;
		}
		else if (i < 19)
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
		drawRoundBtn(135, 55, 290, 95, F("Name"), menuBackground, menuBtnBorder, menuBtnText, CENTER);
		break;
	case 4:
		drawRoundBtn(292, 55, 475, 95, RXtimedMSG.node[node].name, menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
		break;
	case 5:
		drawRoundBtn(135, 99, 259, 139, F("Channel"), menuBackground, menuBtnBorder, menuBtnText, CENTER);
		break;
	case 6:
		drawRoundBtn(135, 143, 180, 183, F("ID"), menuBackground, menuBtnBorder, menuBtnText, CENTER);
		break;
	case 7:
		drawRoundBtn(182, 143, 245, 183, String(RXtimedMSG.node[node].id, 16), menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
		break;
	case 8:
		drawRoundBtn(247, 143, 378, 183, F("Interval"), menuBackground, menuBtnBorder, menuBtnText, CENTER);
		break;
	case 9:
		drawRoundBtn(380, 143, 475, 183, String(RXtimedMSG.node[node].interval), menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
		break;
	case 10:
		drawRoundBtn(135, 187, 475, 223, F("FRAME"), menuBackground, menuBtnBorder, menuBtnText, CENTER);
		break;
	case 11:
		myGLCD.setFont(SmallFont);
		drawRoundBtn(134, 227, 175, 272, " " + String(RXtimedMSG.node[g_var8[POS0]].data[0], 16), menuBtnColor, menuBtnBorder, menuBtnText, LEFT);
		break;
	case 12:
		drawRoundBtn(177, 227, 218, 272, " " + String(RXtimedMSG.node[g_var8[POS0]].data[1], 16), menuBtnColor, menuBtnBorder, menuBtnText, LEFT);
		break;
	case 13:
		drawRoundBtn(220, 227, 261, 272, " " + String(RXtimedMSG.node[g_var8[POS0]].data[2], 16), menuBtnColor, menuBtnBorder, menuBtnText, LEFT);
		break;
	case 14:
		drawRoundBtn(263, 227, 304, 272, " " + String(RXtimedMSG.node[g_var8[POS0]].data[3], 16), menuBtnColor, menuBtnBorder, menuBtnText, LEFT);
		break;
	case 15:
		drawRoundBtn(306, 227, 347, 272, " " + String(RXtimedMSG.node[g_var8[POS0]].data[4], 16), menuBtnColor, menuBtnBorder, menuBtnText, LEFT);
		break;
	case 16:
		drawRoundBtn(349, 227, 390, 272, " " + String(RXtimedMSG.node[g_var8[POS0]].data[5], 16), menuBtnColor, menuBtnBorder, menuBtnText, LEFT);
		break;
	case 17:
		drawRoundBtn(392, 227, 433, 272, " " + String(RXtimedMSG.node[g_var8[POS0]].data[6], 16), menuBtnColor, menuBtnBorder, menuBtnText, LEFT);
		break;
	case 18:
		drawRoundBtn(435, 227, 476, 272, " " + String(RXtimedMSG.node[g_var8[POS0]].data[7], 16), menuBtnColor, menuBtnBorder, menuBtnText, LEFT);
		myGLCD.setFont(BigFont);
		break;
	case 19:
		drawRoundBtn(135, 276, 304, 316, F("Accept"), menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
		break;
	case 20:
		drawRoundBtn(306, 276, 475, 316, F("Cancel"), menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
		break;
	case 21:
		drawSendChannel(RXtimedMSG.node[g_var8[POS0]].channel);
		break;
	case 22:
		return false;
		break;
	}

	graphicLoaderState++;
	return true;
}

void deleteNode(uint8_t node)
{
	//RXtimedMSG.node[node].name = "";
	for (uint8_t i = 0; i < 9; i++)
	{
		RXtimedMSG.node[node].name[i] = '\0';
	}
	RXtimedMSG.node[node].isDel = true;
	RXtimedMSG.node[node].isOn = false;
	RXtimedMSG.node[node].id = 0;
	RXtimedMSG.node[node].interval = 0;
	RXtimedMSG.node[node].channel = 0;
	for (uint8_t i = 0; i < 8; i++)
	{
		RXtimedMSG.node[node].data[i] = 0;
	}
}

void timedTXButtons()	
{
	if (Touch_getXY())
	{
		if ((y >= 55) && (y <= 95) && g_var8[POS4] > 0)
		{
			if ((x >= 133) && (x <= 271))
			{
				waitForIt(133, 55, 271, 95);
				// Send
				TXSend(displayedNodePosition[0]);
			}
			if ((x >= 271) && (x <= 340))
			{
				waitForIt(271, 55, 340, 95);
				// On
				RXtimedMSG.node[displayedNodePosition[0]].isOn = !RXtimedMSG.node[displayedNodePosition[0]].isOn;
				RXtimedMSG.node[displayedNodePosition[0]].isOn ? drawSquareBtn(271, 55, 340, 95, F("ON"), menuBtnColor, menuBtnBorder, menuBtnText, CENTER) : drawSquareBtn(271, 55, 340, 95, F("OFF"), menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
			}
			if ((x >= 340) && (x <= 409))
			{
				waitForIt(340, 55, 409, 95);
				// Edit
				graphicLoaderState = 0;
				g_var8[POS0] = displayedNodePosition[0];
				state = 2;
			}
			if ((x >= 409) && (x <= 477))
			{
				waitForIt(409, 55, 477, 95);
				// Del
				deleteNode(displayedNodePosition[0]);
				saveRXMsg();
				drawTXNode(g_var8[POS3]);
			}
		}
		if ((y >= 100) && (y <= 140) && g_var8[POS4] > 1)
		{
			if ((x >= 133) && (x <= 271))
			{
				waitForIt(133, 100, 271, 140);
				// Send
				TXSend(displayedNodePosition[1]);
			}
			if ((x >= 271) && (x <= 340))
			{
				waitForIt(271, 100, 340, 140);
				// On
				RXtimedMSG.node[displayedNodePosition[1]].isOn = !RXtimedMSG.node[displayedNodePosition[1]].isOn;
				RXtimedMSG.node[displayedNodePosition[1]].isOn ? drawSquareBtn(271, 100, 340, 140, F("ON"), menuBtnColor, menuBtnBorder, menuBtnText, CENTER) : drawSquareBtn(271, 100, 340, 140, F("OFF"), menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
			}
			if ((x >= 340) && (x <= 409))
			{
				waitForIt(340, 100, 409, 140);
				// Edit
				graphicLoaderState = 0;
				g_var8[POS0] = displayedNodePosition[1];
				state = 2;
			}
			if ((x >= 409) && (x <= 477))
			{
				waitForIt(409, 100, 477, 140);
				// Del
				deleteNode(displayedNodePosition[1]);
				saveRXMsg();
				drawTXNode(g_var8[POS3]);
			}
		}
		if ((y >= 145) && (y <= 185) && g_var8[POS4] > 2)
		{
			if ((x >= 133) && (x <= 271))
			{
				waitForIt(133, 145, 271, 185);
				// Send
				TXSend(displayedNodePosition[2]);
			}
			if ((x >= 271) && (x <= 340))
			{
				waitForIt(271, 145, 340, 185);
				// On
				RXtimedMSG.node[displayedNodePosition[2]].isOn = !RXtimedMSG.node[displayedNodePosition[2]].isOn;
				RXtimedMSG.node[displayedNodePosition[2]].isOn ? drawSquareBtn(271, 145, 340, 185, F("ON"), menuBtnColor, menuBtnBorder, menuBtnText, CENTER) : drawSquareBtn(271, 145, 340, 185, F("OFF"), menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
			}
			if ((x >= 340) && (x <= 409))
			{
				waitForIt(340, 145, 409, 185);
				// Edit
				graphicLoaderState = 0;
				g_var8[POS0] = displayedNodePosition[2];
				state = 2;
			}
			if ((x >= 409) && (x <= 477))
			{
				waitForIt(409, 145, 477, 185);
				// Del
				deleteNode(displayedNodePosition[2]);
				saveRXMsg();
				drawTXNode(g_var8[POS3]);
			}
		}
		if ((y >= 190) && (y <= 230) && g_var8[POS4] > 3)
		{
			if ((x >= 133) && (x <= 271))
			{
				waitForIt(133, 190, 271, 230);
				// Send
				TXSend(displayedNodePosition[3]);
			}
			if ((x >= 271) && (x <= 340))
			{
				waitForIt(271, 190, 340, 230);
				// On
				RXtimedMSG.node[displayedNodePosition[3]].isOn = !RXtimedMSG.node[displayedNodePosition[3]].isOn;
				RXtimedMSG.node[displayedNodePosition[3]].isOn ? drawSquareBtn(271, 190, 340, 230, F("ON"), menuBtnColor, menuBtnBorder, menuBtnText, CENTER) : drawSquareBtn(271, 190, 340, 230, F("OFF"), menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
			}
			if ((x >= 340) && (x <= 409))
			{
				waitForIt(340, 190, 409, 230);
				// Edit
				graphicLoaderState = 0;
				g_var8[POS0] = displayedNodePosition[3];
				state = 2;
			}
			if ((x >= 409) && (x <= 477))
			{
				waitForIt(409, 190, 477, 230);
				// Del
				deleteNode(displayedNodePosition[3]);
				saveRXMsg();
				drawTXNode(g_var8[POS3]);
			}
		}
		if ((y >= 235) && (y <= 275) && g_var8[POS4] > 4)
		{
			if ((x >= 133) && (x <= 271))
			{
				waitForIt(133, 235, 271, 275);
				// Send
				TXSend(displayedNodePosition[4]);
			}
			if ((x >= 271) && (x <= 340))
			{
				waitForIt(271, 235, 340, 275);
				// On
				RXtimedMSG.node[displayedNodePosition[4]].isOn = !RXtimedMSG.node[displayedNodePosition[4]].isOn;
				RXtimedMSG.node[displayedNodePosition[4]].isOn ? drawSquareBtn(271, 235, 340, 275, F("ON"), menuBtnColor, menuBtnBorder, menuBtnText, CENTER) : drawSquareBtn(271, 235, 340, 275, F("OFF"), menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
			}
			if ((x >= 340) && (x <= 409))
			{
				waitForIt(340, 235, 409, 275);
				// Edit
				graphicLoaderState = 0;
				g_var8[POS0] = displayedNodePosition[4];
				state = 2;
			}
			if ((x >= 409) && (x <= 477))
			{
				waitForIt(409, 235, 477, 275);
				// Del
				deleteNode(displayedNodePosition[4]);
				saveRXMsg();
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
				if (g_var8[POS3] < 15)
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
		drawTimedTX();
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
	case 14:
		// Name
		g_var8[POS1] = keyboardController(g_var8[POS5]);
		if (g_var8[POS1] == 0xF1) // Accept
		{
			strncpy(RXtimedMSG.node[g_var8[POS0]].name, keyboardInput, 9);
			for (uint8_t i = 0; i < 8; i++)
			{
				keyboardInput[i] = '\0';
			}

			graphicLoaderState = 0;
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
			if ((x >= 292) && (x <= 475))
			{
				waitForIt(292, 55, 475, 95);
				// Set name
				g_var8[POS5] = 0;
				drawkeyboard();
				state = 14;
			}
		}
		if ((y >= 99) && (y <= 139))
		{
			if ((x >= 261) && (x <= 331))
			{
				waitForIt(261, 99, 331, 139);
				// Channel 0
				RXtimedMSG.node[g_var8[POS0]].channel = 0;
				drawSendChannel(RXtimedMSG.node[g_var8[POS0]].channel);
			}
			if ((x >= 333) && (x <= 403))
			{
				waitForIt(333, 99, 403, 139);
				// Channel 1
				RXtimedMSG.node[g_var8[POS0]].channel = 1;
				drawSendChannel(RXtimedMSG.node[g_var8[POS0]].channel);
			}
			if ((x >= 405) && (x <= 475))
			{
				waitForIt(405, 99, 475, 139);
				// WIFI
				RXtimedMSG.node[g_var8[POS0]].channel = 2;
				drawSendChannel(RXtimedMSG.node[g_var8[POS0]].channel);
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
		if ((y >= 227) && (y <= 272))
		{
			if ((x >= 134) && (x <= 175))
			{
				waitForIt(134, 227, 175, 272);
				// Set Data[0]
				g_var8[POS2] = 0;
				g_var16[POS0] = 0;
				resetKeypad();
				state = 6;
				drawKeypad();
			}
			if ((x >= 177) && (x <= 218))
			{
				waitForIt(177, 227, 218, 272);
				// Set Data[1]
				g_var8[POS2] = 0;
				g_var16[POS0] = 0;
				resetKeypad();
				state = 7;
				drawKeypad();
			}
			if ((x >= 220) && (x <= 261))
			{
				waitForIt(220, 227, 261, 272);
				// Set Data[2]
				g_var8[POS2] = 0;
				g_var16[POS0] = 0;
				resetKeypad();
				state = 8;
				drawKeypad();
			}
			if ((x >= 263) && (x <= 304))
			{
				waitForIt(263, 227, 304, 272);
				// Set Data[3]
				g_var8[POS2] = 0;
				g_var16[POS0] = 0;
				resetKeypad();
				state = 9;
				drawKeypad();
			}
			if ((x >= 306) && (x <= 347))
			{
				waitForIt(306, 227, 347, 272);
				// Set Data[4]
				g_var8[POS2] = 0;
				g_var16[POS0] = 0;
				resetKeypad();
				state = 10;
				drawKeypad();
			}
			if ((x >= 350) && (x <= 391))
			{
				waitForIt(349, 227, 390, 272);
				g_var8[POS2] = 0;
				g_var16[POS0] = 0;
				resetKeypad();
				// Set Data[5]
				state = 11;
				drawKeypad();
			}
			if ((x >= 392) && (x <= 433))
			{
				waitForIt(392, 227, 433, 272);
				// Set Data[6]
				g_var8[POS2] = 0;
				g_var16[POS0] = 0;
				resetKeypad();
				state = 12;
				drawKeypad();
			}
			if ((x >= 435) && (x <= 476))
			{
				waitForIt(435, 227, 476, 272);
				// Set Data[7]
				g_var8[POS2] = 0;
				g_var16[POS0] = 0;
				resetKeypad();
				state = 13;
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
				RXtimedMSG.node[g_var8[POS0]].isDel = false;
				saveRXMsg();
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
	CAN_FRAME timedTXFRAME;

	// Generate CAN Bus message for every active struture object and send if interval is ready
	for (uint8_t i = 0; i < 20; i++)
	{
		if (!RXtimedMSG.node[i].isDel && RXtimedMSG.node[i].isOn && (millis() - RXtimedMSG.node[i].timer > RXtimedMSG.node[i].interval))
		{
			timedTXFRAME.id = RXtimedMSG.node[i].id;
			timedTXFRAME.length = 8;
			timedTXFRAME.extended = false;
			for (uint8_t j = 0; j < 8; j++)
			{
				timedTXFRAME.data.bytes[j] = RXtimedMSG.node[i].data[j];
			}
			can1.sendCANOut(RXtimedMSG.node[i].channel, timedTXFRAME, false);
			RXtimedMSG.node[i].timer = millis();
		}
	}
}

void TXSend(uint8_t msgNum)
{
	CAN_FRAME TXFRAME;
	
	TXFRAME.id = RXtimedMSG.node[msgNum].id;
	TXFRAME.length = 8;
	TXFRAME.extended = false;
	for (uint8_t j = 0; j < 8; j++)
	{
		TXFRAME.data.bytes[j] = RXtimedMSG.node[msgNum].data[j];
	}
	can1.sendCANOut(RXtimedMSG.node[msgNum].channel, TXFRAME, false);
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
	graphicLoaderState++;
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
				g_var8[POS0] = 0;
			}
			if ((y >= 95) && (y <= 130))
			{
				waitForItRect(140, 95, 300, 130);
				g_var8[POS0] = 1;
			}
			if ((y >= 130) && (y <= 165))
			{
				waitForItRect(140, 130, 300, 165);
				g_var8[POS0] = 2;
			}
			if ((y >= 165) && (y <= 200))
			{
				waitForItRect(140, 165, 300, 200);
				g_var8[POS0] = 3;
			}
			if ((y >= 200) && (y <= 235))
			{
				waitForItRect(140, 200, 300, 235);
				g_var8[POS0] = 4;
			}
			if ((y >= 235) && (y <= 270))
			{
				waitForItRect(140, 235, 300, 270);
				g_var8[POS0] = 5;
			}
		}
		if ((y >= 275) && (y <= 315))
		{
			if ((x >= 140) && (x <= 300))
			{
				// CAN0
				waitForItRect(140, 275, 300, 315);
				can1.setBaud0(baudRates[g_var8[POS0]]);
				drawCurrentBaud();
			}
			if ((x >= 305) && (x <= 465))
			{
				// CAN21
				waitForItRect(305, 275, 475, 315);
				can1.setBaud1(baudRates[g_var8[POS0]]);
				drawCurrentBaud();
			}
		}
	}
}

// Baud rate is set if found, otherwise the baud will default to 500k
void findBaud()
{
	// Find baud rate is a blocking function, takes about 2-3 seconds to run
	can1.findBaudRate0();
	backgroundProcess();
	can1.findBaudRate1();
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
	case 13:
		return false;
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
				state = 2;
				g_var8[POS1] = 0;
				g_var16[POS0] = 0;
				resetKeypad();
				drawKeypad();
			}
			if ((x >= 345) && (x <= 475))
			{
				waitForIt(345, 125, 475, 175);
				// Set CAN 0Mask
				state = 3;
				g_var8[POS1] = 0;
				g_var16[POS0] = 0;
				resetKeypad();
				drawKeypad();
			}
		}
		if ((y >= 180) && (y <= 230))
		{
			if ((x >= 205) && (x <= 340))
			{
				waitForIt(205, 180, 340, 230);
				// Set CAN1 Filter
				state = 4;
				g_var8[POS1] = 0;
				g_var16[POS0] = 0;
				resetKeypad();
				drawKeypad();
			}
			if ((x >= 345) && (x <= 475))
			{
				waitForIt(345, 180, 475, 230);
				// Set CAN1 Mask
				state = 5;
				g_var8[POS1] = 0;
				g_var16[POS0] = 0;
				resetKeypad();
				drawKeypad();
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

// Set new filter mask
void filterMask()
{
	switch (state)
	{
	case 0:
		drawFilterMask() ? state = 0 : state = 1;
		break;
	case 1:
		filterMaskButtons();
		break;
	case 2: // Set channel 0 filter
		g_var8[POS0] = keypadController(g_var8[POS1], g_var16[POS0]);

		if (g_var8[POS0] == 0xF1) // Accept
		{
			state = 0;
			graphicLoaderState = 0;
			CAN0Filter = g_var16[POS0];
			can1.setFilterMask0(CAN0Filter, CAN0Mask);
		}
		else if (g_var8[POS0] == 0xF0) // Cancel
		{
			state = 0;
			graphicLoaderState = 0;
		}
		break;
	case 3:
		g_var8[POS0] = keypadController(g_var8[POS1], g_var16[POS0]);
		if (g_var8[POS0] == 0xF1) // Accept
		{
			state = 0;
			graphicLoaderState = 0;
			CAN0Mask = g_var16[POS0];
			can1.setFilterMask0(CAN0Filter, CAN0Mask);
		}
		else if (g_var8[POS0] == 0xF0) // Cancel
		{
			state = 0;
			graphicLoaderState = 0;
		}
		break;
	case 4:
		g_var8[POS0] = keypadController(g_var8[POS1], g_var16[POS0]);
		if (g_var8[POS0] == 0xF1) // Accept
		{
			state = 0;
			graphicLoaderState = 0;
			CAN1Filter = g_var16[POS0];
			can1.setFilterMask1(CAN1Filter, CAN1Mask);
		}
		else if (g_var8[POS0] == 0xF0) // Cancel
		{
			state = 0;
			graphicLoaderState = 0;
		}
		break;
	case 5:
		g_var8[POS0] = keypadController(g_var8[POS1], g_var16[POS0]);
		if (g_var8[POS0] == 0xF1) // Accept
		{
			state = 0;
			graphicLoaderState = 0;
			CAN1Mask = g_var16[POS0];
			can1.setFilterMask1(CAN1Filter, CAN1Mask);
		}
		else if (g_var8[POS0] == 0xF0) // Cancel
		{
			state = 0;
			graphicLoaderState = 0;
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
		drawSquareBtn(425, 80, 475, 165, F("/\\"), menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
		break;
	case 3:
		drawSquareBtn(425, 165, 475, 250, F("\\/"), menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
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
		drawSquareBtn(386, 275, 479, 315, F("View"), menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
		break;
	case 8:
		return false;
		break;
	}
	graphicLoaderState++;
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

	// Draw the scroll window
	for (uint8_t i = 0; i < MAXSCROLL; i++)
	{
		if ((g_var8[POS0] + i < 20))
		{
			if (fileList[(g_var8[POS0] + i)][0] != '\0')
			{
				// Max size that will fit in GUI position
				char temp[17];

				// Remove .txt from file name
				char incoming[13];
				strcpy(incoming, fileList[(g_var8[POS0] + i)]);
				char* remove = strtok(incoming, ".");
				strcpy(temp, remove);

				// Split name - size
				strcat(temp, "-");

				// Create size string 
				char buf[5];
				int a = (sdCard.fileSize(fileList[(g_var8[POS0] + i)]) / 1024);
				itoa(a, buf, 10);

				// Put it all together
				strcat(temp, buf);
				strcat(temp, "kb");

				drawSquareBtn(135, y, 420, y + 35, temp, menuBackground, menuBtnBorder, menuBtnText, LEFT);
			}
			else
			{
				char temp[13];
				sprintf(temp, "%s", fileList[(g_var8[POS0] + i)]);
				drawSquareBtn(135, y, 420, y + 35, temp, menuBackground, menuBtnBorder, menuBtnText, LEFT);
			}
		}
		else
		{
			drawSquareBtn(135, y, 420, y + 35, "", menuBackground, menuBtnBorder, menuBtnText, LEFT);
		}
		y = y + 35;
	}
}

//
void logDeleteConfirmation()
{
	char buf[13];
	strcpy(buf, fileList[g_var16[POS0]]);
	strcat(buf, "?");
	drawErrorMSG(F("Confirmation"), F("Delete File"), buf);
}

// Buttons for the capture playback program
void CANLogButtons()
{
	// Touch screen controls
	if (Touch_getXY())
	{
		if ((x >= 135) && (x <= 420))
		{
			if ((y >= 60) && (y <= 95))
			{
				waitForItRect(135, 60, 420, 95);
				g_var16[POS0] = 0 + g_var8[POS0];
			}
			if ((y >= 95) && (y <= 130))
			{
				waitForItRect(135, 95, 420, 130);
				g_var16[POS0] = 1 + g_var8[POS0];
			}
			if ((y >= 130) && (y <= 165))
			{
				waitForItRect(135, 130, 420, 165);
				g_var16[POS0] = 2 + g_var8[POS0];
			}
			if ((y >= 165) && (y <= 200))
			{
				waitForItRect(135, 165, 420, 200);
				g_var16[POS0] = 3 + g_var8[POS0];
			}
			if ((y >= 200) && (y <= 235))
			{
				waitForItRect(135, 200, 420, 235);
				g_var16[POS0] = 4 + g_var8[POS0];
			}
			if ((y >= 235) && (y <= 270))
			{
				waitForItRect(135, 235, 420, 270);
				g_var16[POS0] = 5 + g_var8[POS0];
			}
			/*
			SerialUSB.print("g_var16[POS0]: ");
			SerialUSB.println(g_var16[POS0]);
			SerialUSB.print("g_var8[POS0]: ");
			SerialUSB.println(g_var8[POS0]);
			SerialUSB.print("fileList[g_var16[POS0]]: ");
			SerialUSB.println(fileList[g_var16[POS0]]);
			*/
		}
		if ((x >= 425) && (x <= 475))
		{
			if ((y >= 80) && (y <= 160))
			{
				waitForItRect(425, 80, 475, 165);
				// Scroll up
				if (g_var8[POS0] > 0)
				{
					g_var8[POS0]--;
					drawCANLogScroll();
				}
			}
		}
		if ((x >= 425) && (x <= 475))
		{
			if ((y >= 160) && (y <= 240))
			{
				waitForItRect(425, 165, 475, 250);
				// Scroill down
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

				sdCard.readLogFile(fileLoc);

				drawSquareBtn(131, 275, 216, 315, F("Start"), menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
			}
			if ((x >= 216) && (x <= 301))
			{
				// Delete
				waitForItRect(216, 275, 301, 315);
				SerialUSB.println(fileList[g_var16[POS0]]);
				logDeleteConfirmation();
				state = 1;
			}
			if ((x >= 301) && (x <= 386))
			{
				// Split
				waitForItRect(301, 275, 386, 315);
				uint32_t fileSize = sdCard.fileLength(fileLoc);
				// TODO Find a faster way to do this task

				sdCard.tempCopy(fileLoc);
				sdCard.split("canlog/temp.txt", fileSize);
				sdCard.deleteFile("canlog/temp.txt");
				drawCANLogScroll();
			}
			if ((x >= 386) && (x <= 479))
			{
				// View
				waitForItRect(386, 275, 479, 315);
				sdCard.readLogFileLCD(fileLoc, g_var32[POS0], false);
				state = 2;
			}
		}
	}
}

void playback()
{
	if (state == 0)
	{
		CANLogButtons();
	}
	else if (state == 1)
	{
		char fileLocation[20] = "CANLOG/";
		strcat(fileLocation, fileList[g_var16[POS0]]);
		uint8_t input = errorMSGButton(2);
		switch (input)
		{
		case 1:
			state = 0;
			sdCard.deleteFile(fileLocation);
			for (uint8_t i = 0; i < 10; i++)
			{
				for (uint8_t j = 0; j < 13; j++)
				{
					fileList[i][j] = '\0';
				}
			}
			drawCANLogScroll();
			break;
		case 2:
			state = 0;
			drawCANLogScroll();
			break;
		case 3:
			state = 0;
			drawCANLogScroll();
			break;
		}
	}
	if (state == 2)
	{
		g_var8[POS0] = swipe(g_var32[POS1], g_var8[POS2], g_var16[POS2], g_var16[POS3], g_var16[POS4], g_var16[POS5]);
		if (g_var8[POS0] == SWIPE_DOWN && !Touch_getXY())
		{
			char fileLoc[20] = "CANLOG/";
			strcat(fileLoc, fileList[g_var16[POS0]]);
			sdCard.readLogFileLCD(fileLoc, g_var32[POS0], true);
		}
		if (g_var8[POS0] == SWIPE_UP && !Touch_getXY())
		{
			char fileLoc[20] = "CANLOG/";
			strcat(fileLoc, fileList[g_var16[POS0]]);
			sdCard.readLogFileLCD(fileLoc, g_var32[POS0], false);
		}
		if (g_var8[POS0] == SWIPE_RIGHT && !Touch_getXY())
		{
			state = 3;
			graphicLoaderState = 0;
			g_var8[POS0] = 0;
			g_var32[POS0] = 0;
		}
		if (g_var8[POS0] == SWIPE_LEFT && !Touch_getXY())
		{
			// Looking for work
		}
	}
	if (state == 3)
	{
		if (!drawCANLog())
		{
			state = 0;
			
			drawCANLogScroll();
		}
	}
}