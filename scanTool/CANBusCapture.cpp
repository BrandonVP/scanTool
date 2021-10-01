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
		drawRoundBtn(310, 245, 475, 295, F(""), menuBackground, menuBtnBorder, menuBtnText, CENTER);
		break;
	case 10:
		drawSquareBtn(150, 301, 479, 319, VERSION, themeBackground, themeBackground, menuBtnColor, CENTER);
		break;
	}
}

void CANBusButtons()
{
	// Touch screen controls
	if (myTouch.dataAvailable())
	{
		myTouch.read();
		x = myTouch.getX();
		y = myTouch.getY();

		// Start Scan
		if ((x >= 140) && (x <= 308))
		{
			if ((y >= 80) && (y <= 130))
			{
				waitForIt(140, 80, 305, 130);
				// Capture
				page = 1;
				hasDrawn = false;
			}
			if ((y >= 135) && (y <= 185))
			{
				waitForIt(140, 135, 305, 185);
				// CAN0 RX
				page = 3;
				var4 = 0;
				hasDrawn = false;
			}
			if ((y >= 190) && (y <= 240))
			{
				waitForIt(140, 190, 305, 240);
				// Filter Mask
				page = 4;
				hasDrawn = false;
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
				page = 2;
				hasDrawn = false;
			}
			if ((y >= 135) && (y <= 185))
			{
				waitForIt(310, 135, 475, 185);
				// CAN1 RX
				page = 3;
				var4 = 1;
				hasDrawn = false;
			}
			if ((y >= 190) && (y <= 240))
			{
				waitForIt(310, 190, 475, 240);
				// Baud
				page = 5;
				hasDrawn = false;
			}
			if ((y >= 245) && (y <= 295))
			{
				//waitForIt(310, 245, 475, 295);
				// Unused
			}
		}
	}
}

/*============== CAPTURE ==============*/
void capture()
{

}

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
		drawSquareBtn(310, 200, 470, 240, F("Start"), menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
		break;
	case 3:
		drawSquareBtn(310, 245, 470, 285, F("Stop"), menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
		break;
	case 4:
		drawSquareBtn(150, 301, 479, 319, VERSION, themeBackground, themeBackground, menuBtnColor, CENTER);
		break;
	}
}

void drawCaptureSelected()
{
	switch (graphicLoaderState)
	{
	case 0:
		break;
	case 1:
		drawSquareBtn(310, 55, 475, 85, F("Selected"), menuBackground, menuBtnBorder, menuBtnText, CENTER);
		break;
	case 2:
		drawSquareBtn(310, 85, 475, 125, F("CAN1"), menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
		break;
	case 3:
		drawSquareBtn(310, 125, 475, 165, F("Serial"), menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
		break;
	}
}

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
		drawSquareBtn(135, 55, 300, 85, F("Source"), menuBackground, menuBtnBorder, menuBtnText, CENTER);
		break;
	case 3:
		drawSquareBtn(135, 85, 300, 125, F("CAN0"), menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
		break;
	case 4:
		drawSquareBtn(135, 125, 300, 165, F("CAN1"), menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
		break;
	case 5:
		drawSquareBtn(135, 165, 300, 205, F("CAN0/1"), menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
		break;
	case 6:
		drawSquareBtn(135, 205, 300, 245, F("CAN0/TX1"), menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
		break;
	case 7:
		drawSquareBtn(135, 245, 300, 285, F("Bridge0/1"), menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
		break;
	}
}

void drawCaptureSource()
{
	/*
	switch (graphicLoaderState)
	{
	case 0:
		break;
	case 1:
		drawSquareBtn(131, 55, 479, 319, "", themeBackground, themeBackground, themeBackground, CENTER);
		break;
	case 2:
		drawSquareBtn(135, 55, 300, 85, F("Source"), menuBackground, menuBtnBorder, menuBtnText, CENTER);
		drawSquareBtn(135, 85, 300, 125, F("CAN0"), menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
		drawSquareBtn(135, 125, 300, 165, F("CAN1"), menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
		drawSquareBtn(135, 165, 300, 205, F("CAN0/1"), menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
		drawSquareBtn(135, 205, 300, 245, F("CAN0/TX1"), menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
		drawSquareBtn(135, 245, 300, 285, F("Bridge0/1"), menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
		break;
	case 3:
		
		//drawSquareBtn(245, 55, 360, 85, F("Capture"), menuBackground, menuBtnBorder, menuBtnText, CENTER);
		//drawSquareBtn(245, 85, 360, 125, F("CAN0 RX"), menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
		//drawSquareBtn(245, 125, 360, 165, F("CAN1 RX"), menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
		//drawSquareBtn(245, 165, 360, 205, F("CAN0 TX"), menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
		//drawSquareBtn(245, 205, 360, 245, F("CAN1 TX"), menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
		
		break;
	case 4:
		drawSquareBtn(310, 55, 475, 85, F("Selected"), menuBackground, menuBtnBorder, menuBtnText, CENTER);
		drawSquareBtn(310, 85, 475, 125, F("CAN1"), menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
		drawSquareBtn(310, 125, 475, 165, F("Serial"), menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
		
		//drawSquareBtn(310, 55, 475, 85, F("Output"), menuBackground, menuBtnBorder, menuBtnText, CENTER);
		//drawSquareBtn(310, 85, 475, 125, F("LCD"), menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
		//drawSquareBtn(310, 125, 475, 165, F("Serial"), menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
		//drawSquareBtn(310, 165, 475, 205, F("SD Card"), menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
		//drawSquareBtn(310, 205, 475, 245, F("Back"), menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
		
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
		drawSquareBtn(310, 200, 470, 240, F("Start"), menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
		break;
	case 11:
		drawSquareBtn(310, 245, 470, 285, F("Stop"), menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
		break;
	case 12:
		drawSquareBtn(150, 301, 479, 319, VERSION, themeBackground, themeBackground, menuBtnColor, CENTER);
		break;
	}
*/
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
void sendCANFrame(uint8_t channel)
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

/*============== Baud ==============*/

void drawBaud()
{
	drawSquareBtn(131, 55, 479, 319, "", themeBackground, themeBackground, themeBackground, CENTER);
	drawSquareBtn(140, 275, 300, 315, F("Set CAN0"), menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
	drawSquareBtn(305, 275, 465, 315, F("Set CAN1"), menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
	drawSquareBtn(305, 60, 475, 100, F("CAN0"), menuBackground, menuBtnBorder, menuBtnText, CENTER);
	drawSquareBtn(305, 150, 475, 190, F("CAN1"), menuBackground, menuBtnBorder, menuBtnText, CENTER);
}

void drawCurrentBaud()
{
	drawSquareBtn(305, 100, 475, 140, String(can1.getBaud0()), menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
	drawSquareBtn(305, 190, 475, 230, String(can1.getBaud1()), menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
}

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
				// CAN2
				waitForItRect(305, 275, 465, 315);
				can1.setBaud1(var4);
				drawCurrentBaud();
			}
		}
	}
}

/*============== Filter Mask ==============*/
void drawFilterMask()
{
	drawSquareBtn(131, 55, 479, 319, "", themeBackground, themeBackground, themeBackground, CENTER);

	drawRoundBtn(145, 70, 200, 120, F("CAN"), menuBackground, menuBtnBorder, menuBtnText, CENTER);
	drawRoundBtn(205, 70, 340, 120, F("Filter"), menuBackground, menuBtnBorder, menuBtnText, CENTER);
	drawRoundBtn(345, 70, 475, 120, F("Mask"), menuBackground, menuBtnBorder, menuBtnText, CENTER);

	drawRoundBtn(145, 125, 200, 175, F("0"), menuBackground, menuBtnBorder, menuBtnText, CENTER);
	drawRoundBtn(205, 125, 340, 175, String(CAN0Filter, 16), menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
	drawRoundBtn(345, 125, 475, 175, String(CAN0Mask, 16), menuBtnColor, menuBtnBorder, menuBtnText, CENTER);

	drawRoundBtn(145, 180, 200, 230, F("1"), menuBackground, menuBtnBorder, menuBtnText, CENTER);
	drawRoundBtn(205, 180, 340, 230, String(CAN1Filter, 16), menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
	drawRoundBtn(345, 180, 475, 230, String(CAN1Mask, 16), menuBtnColor, menuBtnBorder, menuBtnText, CENTER);

	drawRoundBtn(145, 235, 475, 285, F("Open All Traffic"), menuBtnColor, menuBtnBorder, menuBtnText, CENTER);

	drawSquareBtn(150, 295, 479, 315, VERSION, themeBackground, themeBackground, menuBtnColor, CENTER);
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
				// Open All Traffic
				can1.startCAN0(0x000, 0x800);
				can1.startCAN1(0x000, 0x800);
			}
		}
	}
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
		state = 0;
		value = var5;
		return 0x11;
	}
	if (var4 == 0x12)
	{
		isFinished = false;
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
		if (!isFinished)
		{
			drawFilterMask();
			isFinished = true;
		}
		filterMaskButtons();
		break;
	case 1: temp = setFilterMask(CAN0Filter);
		if (temp == 0x11)
		{
			can1.setFilterMask0(CAN0Filter, CAN0Mask);
			temp = 0;
		}
		break;
	case 2: temp = setFilterMask(CAN0Mask);
		if (temp == 0x11)
		{
			can1.setFilterMask0(CAN0Filter, CAN0Mask);
			temp = 0;
		}
		break;
	case 3: temp = setFilterMask(CAN1Filter);
		if (temp == 0x11)
		{
			can1.setFilterMask1(CAN1Filter, CAN1Mask);
			temp = 0;
		}
		break;
	case 4: temp = setFilterMask(CAN1Mask);
		if (temp == 0x11)
		{
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

	// Draw the scroll window
	for (int i = 0; i < MAXSCROLL; i++)
	{
		if ((scroll + i < 10))
		{
			char temp[13];
			sprintf(temp, "%s", fileList[scroll + i]);
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
				var1 = 1 + scroll;
			}
			if ((y >= 95) && (y <= 130))
			{
				waitForItRect(150, 95, 410, 130);
				//SerialUSB.println(2 + scroll);
				var1 = 2 + scroll;
			}
			if ((y >= 130) && (y <= 165))
			{
				waitForItRect(150, 130, 410, 165);
				//SerialUSB.println(3 + scroll);
				var1 = 3 + scroll;
			}
			if ((y >= 165) && (y <= 200))
			{
				waitForItRect(150, 165, 410, 200);
				//SerialUSB.println(4 + scroll);
				var1 = 4 + scroll;
			}
			if ((y >= 200) && (y <= 235))
			{
				waitForItRect(150, 200, 410, 235);
				//SerialUSB.println(5 + scroll);
				var1 = 5 + scroll;
			}
			if ((y >= 235) && (y <= 270))
			{
				waitForItRect(150, 235, 410, 270);
				//SerialUSB.println(6 + scroll);
				var1 = 6 + scroll;
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
			strcat(fileLoc, fileList[var1 - 1]);
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
				//sdCard.deleteFile(fileLoc);
			}
			if ((x >= 301) && (x <= 386))
			{
				// Split
				waitForItRect(301, 275, 386, 315);
				uint32_t temp = sdCard.fileLength(fileLoc);
				sdCard.tempCopy(fileLoc);
				sdCard.split("canlog/temp.txt", temp);
				sdCard.deleteFile("canlog/temp.txt");
			}
			if ((x >= 386) && (x <= 479))
			{
				// Settings
				waitForItRect(386, 275, 479, 315);
				sdCard.readLogFile(fileList[var1]);
			}
		}
	}
}