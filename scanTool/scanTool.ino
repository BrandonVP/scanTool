/*
 Name:    scanTool.ino
 Created: 11/15/2020 8:27:18 AM
 Author:  Brandon Van Pelt
*/

/*=========================================================
	Todo List
===========================================================
Read Vehicle DTCs
Read / Clear RZR DTCs
Replace uint8_t scroll with var

8 Var array[9], extra var for bit lock
released when old page != page, switch to find which vars to release
===========================================================
	End Todo List
=========================================================*/

#include "ExtraFunctions.h"
#include "PolarisTools.h"
#include "common.h"
#include "CANBus.h"
#include "definitions.h"
#include "CANBusCapture.h"
#include <malloc.h>
#include <memorysaver.h>
#include <SD.h>
#include "SDCard.h"
#include "Settings.h"
#include <SPI.h>
#include <string.h>
#include <UTouchCD.h>
#include <UTFT.h>
#include <UTouch.h>
#include "VehicleTools.h"

// Harware Objects
CANBus can1;
SDCard sdCard;
DS3231 rtc(SDA, SCL);

// LCD display
//(byte model, int RS, int WR, int CS, int RST, int SER)
UTFT myGLCD(ILI9488_16, 7, 38, 9, 10);
//RTP: byte tclk, byte tcs, byte din, byte dout, byte irq
UTouch  myTouch(2, 6, 3, 4, 5);

// For touch controls
int x, y;

// Used for page control
uint8_t controlPage = 0;
uint8_t page = 0;
bool hasDrawn = false;

// *Used by background process*
uint8_t selectedChannelOut = 0;
uint8_t selectedSourceOut = 0;
uint32_t timer2 = 0;
bool isSerialOut = false;

// General use variables
// Any non-background process function can use
// Initialize to 0 before use
bool nextState = false;
bool isFinished = false;
uint8_t state = 0;
int16_t counter1 = 0;
uint16_t var1 = 0;
uint8_t var2 = 0;
uint16_t var3 = 0;
uint32_t var4 = 0;
uint32_t var5 = 0;
uint32_t timer1 = 0;

// Used for converting keypad input to appropriate hex place
const uint32_t hexTable[8] = { 1, 16, 256, 4096, 65536, 1048576, 16777216, 268435456 };

const uint32_t baudRates[6] = { 1000000, 800000, 500000, 250000, 125000, 100000 };

// TODO: This uses a lot of memory for a simple graphic function
uint32_t waitForItTimer = 0;
uint16_t x1_ = 0;
uint16_t y1_ = 0;
uint16_t x2_ = 0;
uint16_t y2_ = 0;
bool isWaitForIt = false;

// Filter range / Filter Mask
uint32_t CAN0Filter = 0x000;
uint32_t CAN0Mask = 0xFFF;
uint32_t CAN1Filter = 0x000;
uint32_t CAN1Mask = 0xFFF;

// Holds CAN Bus capture replay filenames
char fileList[10][13];

// Determines if a PID scan was performed before displaying pid list
bool hasPID = false;

// Holds PIDS for the pidscan function
uint8_t arrayIn[80];

// TODO: Replace scroll with var
uint8_t scroll = 0;

// Use to load pages in pieces to prevent blocking while loading entire page
uint8_t graphicLoaderState = 0;

/*
Uncomment to update the clock then comment out and upload to
the device a second time to prevent updating time to last time
device was on every at every startup.
*/
//#define UPDATE_CLOCK

/*=========================================================
	Framework Functions
===========================================================*/
//
void bmpDraw(char* filename, int x, int y) {
	File     bmpFile;
	int      bmpWidth, bmpHeight;   // W+H in pixels
	uint8_t  bmpDepth;              // Bit depth (currently must be 24)
	uint32_t bmpImageoffset;        // Start of image data in file
	uint32_t rowSize;               // Not always = bmpWidth; may have padding
	uint8_t  sdbuffer[3 * BUFFPIXEL]; // pixel in buffer (R+G+B per pixel)
	uint16_t lcdbuffer[BUFFPIXEL];  // pixel out buffer (16-bit per pixel)
	uint8_t  buffidx = sizeof(sdbuffer); // Current position in sdbuffer
	boolean  goodBmp = false;       // Set to true on valid header parse
	boolean  flip = true;        // BMP is stored bottom-to-top
	int      w, h, row, col;
	uint8_t  r, g, b;
	uint32_t pos = 0, startTime = millis();
	uint8_t  lcdidx = 0;
	boolean  first = true;

	int dispx = myGLCD.getDisplayXSize();
	int dispy = myGLCD.getDisplayYSize();

	if ((x >= dispx) || (y >= dispy)) return;

	Serial.println();
	Serial.print(F("Loading image '"));
	Serial.print(filename);
	Serial.println('\'');

	// Open requested file on SD card
	if ((bmpFile = SD.open(filename)) == NULL) {
		Serial.println(F("File not found"));
		return;
	}


	// Parse BMP header
	if (read16(bmpFile) == 0x4D42) { // BMP signature

		Serial.println(read32(bmpFile));
		(void)read32(bmpFile); // Read & ignore creator bytes
		bmpImageoffset = read32(bmpFile); // Start of image data
		Serial.print(F("Image Offset: "));
		Serial.println(bmpImageoffset, DEC);

		// Read DIB header
		Serial.print(F("Header size: "));
		Serial.println(read32(bmpFile));
		bmpWidth = read32(bmpFile);
		bmpHeight = read32(bmpFile);

		if (read16(bmpFile) == 1) { // # planes -- must be '1'
			bmpDepth = read16(bmpFile); // bits per pixel
			Serial.print(F("Bit Depth: "));
			Serial.println(bmpDepth);
			if ((bmpDepth == 24) && (read32(bmpFile) == 0)) { // 0 = uncompressed
				goodBmp = true; // Supported BMP format -- proceed!
				Serial.print(F("Image size: "));
				Serial.print(bmpWidth);
				Serial.print('x');
				Serial.println(bmpHeight);

				// BMP rows are padded (if needed) to 4-byte boundary
				rowSize = (bmpWidth * 3 + 3) & ~3;

				// If bmpHeight is negative, image is in top-down order.
				// This is not canon but has been observed in the wild.
				if (bmpHeight < 0) {
					bmpHeight = -bmpHeight;
					flip = false;
				}

				// Crop area to be loaded
				w = bmpWidth;
				h = bmpHeight;
				if ((x + w - 1) >= dispx)  w = dispx - x;
				if ((y + h - 1) >= dispy) h = dispy - y;

				// Set TFT address window to clipped image bounds

				for (row = 0; row < h; row++) { // For each scanline...
				  // Seek to start of scan line.  It might seem labor-
				  // intensive to be doing this on every line, but this
				  // method covers a lot of gritty details like cropping
				  // and scanline padding.  Also, the seek only takes
				  // place if the file position actually needs to change
				  // (avoids a lot of cluster math in SD library).
					if (flip) // Bitmap is stored bottom-to-top order (normal BMP)
						pos = bmpImageoffset + (bmpHeight - 1 - row) * rowSize;
					else     // Bitmap is stored top-to-bottom
						pos = bmpImageoffset + row * rowSize;
					if (bmpFile.position() != pos) { // Need seek?
						bmpFile.seek(pos);
						buffidx = sizeof(sdbuffer); // Force buffer reload
					}

					for (col = 0; col < w; col++) { // For each column...
					  // Time to read more pixel data?
						if (buffidx >= sizeof(sdbuffer)) { // Indeed
						  // Push LCD buffer to the display first
							if (lcdidx > 0) {
								myGLCD.setColor(lcdbuffer[lcdidx]);
								myGLCD.drawPixel(col + 155, row + 1);
								lcdidx = 0;
								first = false;
							}

							bmpFile.read(sdbuffer, sizeof(sdbuffer));
							buffidx = 0; // Set index to beginning
						}

						// Convert pixel from BMP to TFT format
						b = sdbuffer[buffidx++];
						g = sdbuffer[buffidx++];
						r = sdbuffer[buffidx++];
						myGLCD.setColor(r, g, b);
						myGLCD.drawPixel(col + 155, row + 1);
					} // end pixel

				} // end scanline

				// Write any remaining data to LCD
				if (lcdidx > 0) {
					myGLCD.setColor(lcdbuffer[lcdidx]);
					myGLCD.drawPixel(col + 155, row + 1);
				}
			} // end goodBmp
		}
	}

	bmpFile.close();
	if (!goodBmp) Serial.println(F("BMP format not recognized."));

}

// These read 16- and 32-bit types from the SD card file.
// BMP data is stored little-endian, Arduino is little-endian too.
// May need to reverse subscript order if porting elsewhere.
uint16_t read16(File f) {
	uint16_t result;
	((uint8_t*)&result)[0] = f.read(); // LSB
	((uint8_t*)&result)[1] = f.read(); // MSB
	return result;
}
uint32_t read32(File f) {
	uint32_t result;
	((uint8_t*)&result)[0] = f.read(); // LSB
	((uint8_t*)&result)[1] = f.read();
	((uint8_t*)&result)[2] = f.read();
	((uint8_t*)&result)[3] = f.read(); // MSB
	return result;
}

// Custom bitmap
void print_icon(int x, int y, const unsigned char icon[]) {
	myGLCD.setColor(menuBtnColor);
	myGLCD.setBackColor(themeBackground);
	int i = 0, row, column, bit, temp;
	int constant = 1;
	for (row = 0; row < 40; row++) {
		for (column = 0; column < 5; column++) {
			temp = icon[i];
			for (bit = 7; bit >= 0; bit--) {

				if (temp & constant) {
					myGLCD.drawPixel(x + (column * 8) + (8 - bit), y + row);
				}
				else {

				}
				temp >>= 1;
			}
			i++;
		}
	}
}


extern char _end;
extern "C" char* sbrk(int i);
// https://forum.arduino.cc/t/getting-heap-size-stack-size-and-free-ram-from-due/678195/5
// Display used memory
void saveRamStates(uint32_t MaxUsedHeapRAM, uint32_t MaxUsedStackRAM, uint32_t MaxUsedStaticRAM, uint32_t MinfreeRAM)
{
	char* ramstart = (char*)0x20070000;
	char* ramend = (char*)0x20088000;

	char* heapend = sbrk(0);
	register char* stack_ptr asm("sp");
	struct mallinfo mi = mallinfo();
	if (MaxUsedStaticRAM < &_end - ramstart)
	{
		MaxUsedStaticRAM = &_end - ramstart;
	}
	if (MaxUsedHeapRAM < mi.uordblks)
	{
		MaxUsedHeapRAM = mi.uordblks;
	}
	if (MaxUsedStackRAM < ramend - stack_ptr)
	{
		MaxUsedStackRAM = ramend - stack_ptr;
	}
	if (MinfreeRAM > stack_ptr - heapend + mi.fordblks || MinfreeRAM == 0)
	{
		MinfreeRAM = stack_ptr - heapend + mi.fordblks;
	}

	drawSquareBtn(131, 55, 479, 319, "", themeBackground, themeBackground, themeBackground, CENTER);
	drawRoundBtn(135, 80, 310, 130, F("Used RAM"), menuBackground, menuBtnBorder, menuBtnText, CENTER);
	drawRoundBtn(315, 80, 475, 130, String(MaxUsedStaticRAM), menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
	drawRoundBtn(135, 135, 310, 185, F("Used HEAP"), menuBackground, menuBtnBorder, menuBtnText, CENTER);
	drawRoundBtn(315, 135, 475, 185, String(MaxUsedHeapRAM), menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
	drawRoundBtn(135, 190, 310, 240, F("Used STACK"), menuBackground, menuBtnBorder, menuBtnText, CENTER);
	drawRoundBtn(315, 190, 475, 240, String(MaxUsedStackRAM), menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
	drawRoundBtn(135, 245, 310, 295, F("FREE RAM"), menuBackground, menuBtnBorder, menuBtnText, CENTER);
	drawRoundBtn(315, 245, 475, 295, String(MinfreeRAM), menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
	drawSquareBtn(150, 300, 479, 319, VERSION, themeBackground, themeBackground, menuBtnColor, CENTER);
}


/*****************************************************
*           Draw Round/Square Button                 *
*                                                    *
*  Description:   Draws shapes with/without text     *
*  Parameters: int: x start, y start, x stop, y stop *
*              String: Button text                   *
*              Hex value: Background Color           *
*              Hex value: Border of shape            *
*              Hex value: Color of text              *
*              int: Alignment of text #defined as    *
*                   LEFT, CENTER, RIGHT              *
*****************************************************/
void drawRoundBtn(int x_start, int y_start, int x_stop, int y_stop, String button, int backgroundColor, int btnBorderColor, int btnTxtColor, int align) {
	int size, temp, offset;

	myGLCD.setColor(backgroundColor);
	myGLCD.fillRoundRect(x_start, y_start, x_stop, y_stop); // H_Start, V_Start, H_Stop, V_Stop
	myGLCD.setColor(btnBorderColor);
	myGLCD.drawRoundRect(x_start, y_start, x_stop, y_stop);
	myGLCD.setColor(btnTxtColor); // text color
	myGLCD.setBackColor(backgroundColor); // text background
	switch (align)
	{
	case 1:
		myGLCD.print(button, x_start + 5, y_start + ((y_stop - y_start) / 2) - 8); // hor, ver
		break;
	case 2:
		size = button.length();
		temp = ((x_stop - x_start) / 2);
		offset = x_start + (temp - (8 * size));
		myGLCD.print(button, offset, y_start + ((y_stop - y_start) / 2) - 8); // hor, ver
		break;
	case 3:
		// Currently hotwired for deg text only
		myGLCD.print(button, x_start + 55, y_start + ((y_stop - y_start) / 2) - 8); // hor, ver
		break;
	default:
		break;
	}
}
void drawSquareBtn(int x_start, int y_start, int x_stop, int y_stop, String button, int backgroundColor, int btnBorderColor, int btnTxtColor, int align) {
	int size, temp, offset;
	myGLCD.setColor(backgroundColor);
	myGLCD.fillRect(x_start, y_start, x_stop, y_stop); // H_Start, V_Start, H_Stop, V_Stop
	myGLCD.setColor(btnBorderColor);
	myGLCD.drawRect(x_start, y_start, x_stop, y_stop);
	myGLCD.setColor(btnTxtColor); // text color
	myGLCD.setBackColor(backgroundColor); // text background
	switch (align)
	{
	case 1:
		myGLCD.print(button, x_start + 5, y_start + ((y_stop - y_start) / 2) - 8); // hor, ver
		break;
	case 2:
		size = button.length();
		temp = ((x_stop - x_start) / 2);
		offset = x_start + (temp - (8 * size));
		myGLCD.print(button, offset, y_start + ((y_stop - y_start) / 2) - 8); // hor, ver
		break;
	case 3:
		//align left
		break;
	default:
		break;
	}
}

// Function complete load bar
void loadBar(int progress)
{
	if (progress >= DONE)
	{
		drawSquareBtn(200, 200, 400, 220, F("Finished"), menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
		//myGLCD.setColor(menuBtnColor);
		//myGLCD.fillRect(200, 200, 400, 220);

		return;
	}
	drawSquareBtn(199, 199, 401, 221, "", themeBackground, menuBtnBorder, menuBtnText, LEFT);
	myGLCD.setColor(menuBtnColor);
	myGLCD.fillRect(200, 200, (200 + (progress * 25)), 220);
}

// Function to highlight buttons when clicked
void waitForItMenu(int x1, int y1, int x2, int y2)
{
	waitForItTimer = millis();

	if (!isWaitForIt)
	{
		myGLCD.setColor(themeBackground);
		myGLCD.drawRoundRect(x1, y1, x2, y2);
		x1_ = x1;
		y1_ = y1;
		x2_ = x2;
		y2_ = y2;
		isWaitForIt = true;
	}
}
void waitForIt(int x1, int y1, int x2, int y2)
{
	myGLCD.setColor(themeBackground);
	myGLCD.drawRoundRect(x1, y1, x2, y2);
	while (myTouch.dataAvailable())
		myTouch.read();
	myGLCD.setColor(menuBtnBorder);
	myGLCD.drawRoundRect(x1, y1, x2, y2);
}
void waitForItRect(int x1, int y1, int x2, int y2)
{
	myGLCD.setColor(themeBackground);
	myGLCD.drawRect(x1, y1, x2, y2);
	while (myTouch.dataAvailable())
		myTouch.read();
	myGLCD.setColor(menuBtnBorder);
	myGLCD.drawRect(x1, y1, x2, y2);
}

//Only called once at startup to draw the menu
void drawMenu()
{
	// Draw Layout
	drawSquareBtn(0, 0, 479, 319, "", themeBackground, themeBackground, themeBackground, CENTER);
	drawSquareBtn(0, 0, 130, 319, "", menuBackground, menuBackground, menuBackground, CENTER);

	// Draw Menu Buttons
	drawRoundBtn(5, 32, 125, 83, F("CANBUS"), menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
	drawRoundBtn(5, 88, 125, 140, F("VEHTOOL"), menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
	drawRoundBtn(5, 145, 125, 197, F("VEHTOOL"), menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
	drawRoundBtn(5, 202, 125, 254, F("TESTING"), menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
	drawRoundBtn(5, 259, 125, 312, F("SETTING"), menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
}

//Manages the different App pages
void pageControl()
{
	// Check menu buttons for input
	menuButtons();
	switch (page)
	{
	case 0: /*========== CANBUS ==========*/
		if (!hasDrawn)
		{
			// Draw Page
			if (graphicLoaderState < 11)
			{
				drawCANBus();
				graphicLoaderState++;
			}
			else
			{
				hasDrawn = true;
			}
		}
		// Call buttons if any
		CANBusButtons();
		break;
	case 1: // Capture
		switch (state)
		{
		case 0:
			if (!hasDrawn && graphicLoaderState < 8)
			{
				drawCapture();
				graphicLoaderState++;
			}
			else
			{
				state = 1;
				graphicLoaderState = 0;
				drawCaptureSelected();
			}
			break;
		case 1:
			if (!hasDrawn && graphicLoaderState < 8)
			{
				drawCaptureSource();
				graphicLoaderState++;
			}
			else
			{
				hasDrawn = true;
			}
			break;
		case 2:
			if (!hasDrawn && state == 2)
			{
				if (graphicLoaderState < 7)
				{
					drawCaptureOutput();
					graphicLoaderState++;
				}
				else
				{
					hasDrawn = true;
					drawCaptureSelected();
				}
			}
			break;
		}
		// Call buttons if any
		CaptureButtons();
		break;
	case 2: // Playback
		if (!hasDrawn)
		{
			scroll = 0;
			var1 = 0;
			hasDrawn = true;
			// Draw Page
			drawCANLog();
			drawCANLogScroll();
		}
		// Call buttons if any
		CANLogButtons();
		break;
	case 3: // Send CAN Frame
		if (!hasDrawn)
		{
			hasDrawn = true;
			// Draw Page

			// Initialize global var to 0
			var1 = 0;
			var2 = 0;
			var3 = 0;
			state = 0;
			counter1 = 1;
			isFinished = false;
		}
		// Call buttons if any
		sendCANFrame(var4);
		break;
	case 4: // Filter Mask
		if (!hasDrawn)
		{
			hasDrawn = true;
			var1 = 0;
			var4 = 0;
			state = 0;
			counter1 = 5;
			isFinished = false;
		}
		filterMask();
		break;
	case 5: // Set Baud
		if (!hasDrawn)
		{
			// Draw Page
			var4 = 0;
			drawBaud();
			drawBaudScroll();
			drawCurrentBaud();
			hasDrawn = true;
		}
		// Call buttons if any
		baudButtons();
		break;
	case 6: // SD Capture
		if (!hasDrawn)
		{
			hasDrawn = true;
			// Draw Page
			//TODO: Write function
		}
		// Call buttons if any
		break;
	case 7: // LCD Capture
		if (!hasDrawn)
		{
			hasDrawn = true;
			// Draw Page
			drawReadInCANLCD();
			isSerialOut = false;
		}
		// Call buttons if any
		readInCANMsg(selectedChannelOut);
		break;
	case 8: // Serial Capture
		if (!hasDrawn)
		{
			hasDrawn = true;
			// Draw Page
		}
		// Call buttons if any
		break;

	case 9: /*========== VEHTOOL ==========*/
		if (!hasDrawn)
		{
			// Draw Page
			if (graphicLoaderState < 11)
			{
				drawVehicleTools();
				graphicLoaderState++;
			}
			else
			{
				hasDrawn = true;
			}
		}
		// Call buttons if any
		VehicleToolButtons();
		break;
	case 10: // PIDSCAN
		if (!hasDrawn)
		{
			// Draw Page
			hasDrawn = true;
			can1.startCAN0(0x7E0, 0x7EF);
			drawPIDSCAN();
		}
		// Call buttons if any
		startPIDSCAN();
		break;
	case 11: // PIDSTRM
		if (!hasDrawn)
		{
			// Draw Page
			if (hasPID == true)
			{
				can1.startCAN0(0x7E0, 0x7EF);
				for (int i = 0; i < 100; i++)
				{
					arrayIn[i] = 0x00;
				}
				sdCard.readFile(can1.getFullDir(), arrayIn);
				scroll = 0;
				drawPIDStream();
				hasDrawn = true;
			}
			else
			{
				controlPage = 9;
				drawErrorMSG("Error", "Please Perform", "PIDSCAN First");
				hasDrawn = true;
			}
		}
		// Call buttons if any
		if (hasPID == true)
		{
			PIDStreamButtons();
		}
		else
		{
			errorMSGButton(9);
		}
		if ((state == 1) && (counter1 < PIDSAMPLES) && (millis() - timer1 > 1000))
		{
			can1.PIDStream(CAN_PID_ID, arrayIn[var1], true);
			counter1++;
			drawErrorMSG("Samples", String(counter1), "Saved to SD");
			timer1 = millis();
		}
		if ((counter1 == PIDSAMPLES) && (state == 1) && (millis() - timer1 > 2000))
		{
			state = 0;
			scroll = 0;
			drawPIDStream();
		}
		break;
	case 12:
		if (!hasDrawn)
		{
			// Draw Page
			hasDrawn = true;
			//can1.startPID();
		}
		// Call buttons if any
		PIDGauges();
		break;
	case 13:
		if (!hasDrawn)
		{
			// Draw Page
			hasDrawn = true;
			drawVIN();
		}
		// Call buttons if any
		break;
	case 14: // 
		if (!hasDrawn)
		{
			// Draw Page
			hasDrawn = true;
		}
		// Call buttons if any
		break;
	case 15:
		if (!hasDrawn)
		{
			// Draw Page
			hasDrawn = true;
		}
		clearDTC();
		// Call buttons if any
		break;
	case 16:
		if (!hasDrawn)
		{
			// Draw Page
			hasDrawn = true;
		}
		// Call buttons if any
		break;
	case 17:
		if (!hasDrawn)
		{
			// Draw Page
			hasDrawn = true;
		}
		// Call buttons if any
		break;

	case 18: /*========== RZRTOOL ==========*/
		// Draw Page
		if (!hasDrawn)
		{
			if (graphicLoaderState < 11)
			{
				drawRZRTOOL();
				graphicLoaderState++;
			}
			else
			{
				hasDrawn = true;
			}
		}
		// Call buttons if any
		RZRToolButtons();
		break;
	case 19:
		if (!hasDrawn)
		{
			hasDrawn = true;
			// Draw Page
		}
		// Call buttons if any
		break;
	case 20:
		if (!hasDrawn)
		{
			hasDrawn = true;
			// Draw Page
		}
		// Call buttons if any
		break;
	case 21:
		if (!hasDrawn)
		{
			hasDrawn = true;
			// Draw Page
		}
		// Call buttons if any
		break;
	case 22:
		if (!hasDrawn)
		{
			hasDrawn = true;
			// Draw Page
		}
		// Call buttons if any
		break;
	case 23:
		if (!hasDrawn)
		{
			hasDrawn = true;
			// Draw Page
		}
		// Call buttons if any
		break;
	case 24:
		if (!hasDrawn)
		{
			hasDrawn = true;
			// Draw Page
		}
		// Call buttons if any
		break;
	case 25:
		if (!hasDrawn)
		{
			hasDrawn = true;
			// Draw Page
		}
		// Call buttons if any
		break;
	case 26:
		if (!hasDrawn)
		{
			hasDrawn = true;
			// Draw Page
		}
		// Call buttons if any
		break;

	case 27: /*========== EXTRAFN ==========*/
		// Draw Page
		if (!hasDrawn)
		{
			if (graphicLoaderState < 11)
			{
				drawExtraFN();
				graphicLoaderState++;
			}
			else
			{
				hasDrawn = true;
			}
		}
		// Call buttons if any
		extraFNButtons();
		break;
	case 28:
		if (!hasDrawn)
		{
			// Draw Page
			hasDrawn = true;
		}
		// Call buttons if any
		break;
	case 29:
		if (!hasDrawn)
		{
			// Draw Page
			hasDrawn = true;
		}
		// Call buttons if any
		break;
	case 30:
		if (!hasDrawn)
		{
			// Draw Page
			hasDrawn = true;
		}
		// Call buttons if any
		break;
	case 31:
		if (!hasDrawn)
		{
			// Draw Page
			hasDrawn = true;
		}
		// Call buttons if any
		break;
	case 32:
		if (!hasDrawn)
		{
			hasDrawn = true;
			// Draw Page
		}
		// Call buttons if any
		break;
	case 33:
		if (!hasDrawn)
		{
			// Draw Page
			hasDrawn = true;
		}
		// Call buttons if any
		break;
	case 34: // Ford Dongle
		if (!hasDrawn)
		{
			// Draw Page
			hasDrawn = true;
			timer2 = 0;
		}
		if (graphicLoaderState < 19)
		{
			drawDongleSimFord();
			graphicLoaderState++;
		}
		// Call buttons if any
		dongleSimButtonsFord();
		break;
	case 35: // GM Dongle
		if (!hasDrawn)
		{
			hasDrawn = true;
			// Draw Page
			timer2 = 0;
		}
		if (graphicLoaderState < 19)
		{
			drawDongleSim();
			graphicLoaderState++;
		}
		// Call buttons if any
		dongleSimButtons();
		break;

	case 36: /*========== SETTINGS ==========*/
		if (!hasDrawn)
		{
			// Draw Page
			if (graphicLoaderState < 11)
			{
				drawSettings();
				graphicLoaderState++;
			}
			else
			{
				hasDrawn = true;
			}
		}
		// Call buttons if any
		settingsButtons();
		break;
	case 37:
		if (!hasDrawn)
		{
			// Draw Page
			memoryUse();
			hasDrawn = true;
		}
		// Call buttons if any
		break;
	case 38:
		if (!hasDrawn)
		{
			// Draw Page
			hasDrawn = true;
		}
		// Call buttons if any
		break;
	case 39:
		if (!hasDrawn)
		{
			// Draw Page
			hasDrawn = true;
		}
		// Call buttons if any
		break;
	case 40:
		if (!hasDrawn)
		{
			// Draw Page
			hasDrawn = true;
		}
		// Call buttons if any
		break;
	case 41:
		if (!hasDrawn)
		{
			// Draw Page
			hasDrawn = true;
		}
		// Call buttons if any
		break;
	case 42:
		if (!hasDrawn)
		{
			// Draw Page
			hasDrawn = true;
		}
		// Call buttons if any
		break;
	case 43:
		if (!hasDrawn)
		{
			// Draw Page
			hasDrawn = true;
		}
		// Call buttons if any

		break;
	case 44:
		if (!hasDrawn)
		{
			// Draw Page
			hasDrawn = true;
		}
		// Call buttons if any
		break;
	}
}

// User input keypad
void drawKeypad()
{
	drawSquareBtn(145, 55, 479, 319, "", themeBackground, themeBackground, themeBackground, CENTER);
	drawSquareBtn(180, 57, 460, 77, F("Keypad"), themeBackground, themeBackground, menuBtnColor, CENTER);

	uint16_t posY = 80;
	uint8_t numPad = 0x00;

	for (uint8_t i = 0; i < 3; i++)
	{
		int posX = 145;
		for (uint8_t j = 0; j < 6; j++)
		{
			if (numPad < 0x10)
			{
				drawRoundBtn(posX, posY, posX + 50, posY + 40, String(numPad, HEX), menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
				posX += 55;
				numPad++;
			}
		}
		posY += 45;
	}
	drawRoundBtn(365, 170, 470, 210, F("Clear"), menuBtnColor, menuBtnBorder, menuBtnText, CENTER);

	drawRoundBtn(145, 270, 305, 310, F("Accept"), menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
	drawRoundBtn(315, 270, 470, 310, F("Cancel"), menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
}

// User input keypad
int keypadButtons()
{
	// Touch screen controls
	if (myTouch.dataAvailable())
	{
		myTouch.read();
		x = myTouch.getX();
		y = myTouch.getY();

		if ((y >= 80) && (y <= 120))
		{
			// 0
			if ((x >= 145) && (x <= 195))
			{
				waitForIt(145, 80, 195, 120);
				return 0x00;
			}
			// 1
			if ((x >= 200) && (x <= 250))
			{
				waitForIt(200, 80, 250, 120);
				return 0x01;
			}
			// 2
			if ((x >= 255) && (x <= 305))
			{
				waitForIt(255, 80, 305, 120);
				return 0x02;
			}
			// 3
			if ((x >= 310) && (x <= 360))
			{
				waitForIt(310, 80, 360, 120);
				return 0x03;
			}
			// 4
			if ((x >= 365) && (x <= 415))
			{
				waitForIt(365, 80, 415, 120);
				return 0x04;
			}
			// 5
			if ((x >= 420) && (x <= 470))
			{
				waitForIt(420, 80, 470, 120);
				return 0x05;
			}
		}
		if ((y >= 125) && (y <= 165))
		{
			// 6
			if ((x >= 145) && (x <= 195))
			{
				waitForIt(145, 125, 195, 165);
				return 0x06;
			}
			// 7
			if ((x >= 200) && (x <= 250))
			{
				waitForIt(200, 125, 250, 165);
				return 0x07;
			}
			// 8
			if ((x >= 255) && (x <= 305))
			{
				waitForIt(255, 125, 305, 165);
				return 0x08;
			}
			// 9
			if ((x >= 310) && (x <= 360))
			{
				waitForIt(310, 125, 360, 165);
				return 0x09;
			}
			// A
			if ((x >= 365) && (x <= 415))
			{
				waitForIt(365, 125, 415, 165);
				return 0x0A;
			}
			// B
			if ((x >= 420) && (x <= 470))
			{
				waitForIt(420, 125, 470, 165);
				return 0x0B;
			}
		}
		if ((y >= 170) && (y <= 210))
		{
			// C
			if ((x >= 145) && (x <= 195))
			{
				waitForIt(145, 170, 195, 210);
				return 0x0C;
			}
			// D
			if ((x >= 200) && (x <= 250))
			{
				waitForIt(200, 170, 250, 210);
				return 0x0D;
			}
			// E
			if ((x >= 255) && (x <= 305))
			{
				waitForIt(255, 170, 305, 210);
				return 0x0E;
			}
			// F
			if ((x >= 310) && (x <= 360))
			{
				waitForIt(310, 170, 360, 210);
				return 0x0F;

			}
			// Clear
			if ((x >= 365) && (x <= 470))
			{
				waitForIt(365, 170, 470, 210);
				return 0x10;
			}
		}
		if ((y >= 270) && (y <= 310))
		{
			// Accept
			if ((x >= 145) && (x <= 305))
			{
				waitForIt(145, 270, 305, 310);
				return 0x11;

			}
			// Cancel
			if ((x >= 315) && (x <= 470))
			{
				waitForIt(315, 270, 470, 310);
				return 0x12;
			}
		}
	}
	return 0xFF;
}

// Error Message function
void drawErrorMSG(String title, String eMessage1, String eMessage2)
{
	drawSquareBtn(145, 100, 415, 220, "", menuBackground, menuBtnColor, menuBtnColor, CENTER);
	drawSquareBtn(145, 100, 415, 130, title, themeBackground, menuBtnColor, menuBtnBorder, LEFT);
	drawSquareBtn(146, 131, 414, 155, eMessage1, menuBackground, menuBackground, menuBtnText, CENTER);
	drawSquareBtn(146, 155, 414, 180, eMessage2, menuBackground, menuBackground, menuBtnText, CENTER);
	drawRoundBtn(365, 100, 415, 130, "X", menuBtnColor, menuBtnColor, menuBtnText, CENTER);
	drawRoundBtn(155, 180, 275, 215, "Confirm", menuBtnColor, menuBtnColor, menuBtnText, CENTER);
	drawRoundBtn(285, 180, 405, 215, "Cancel", menuBtnColor, menuBtnColor, menuBtnText, CENTER);
}

// Error Message buttons
uint8_t errorMSGButton(uint8_t returnPage)
{
	// Touch screen controls
	if (myTouch.dataAvailable())
	{
		myTouch.read();
		x = myTouch.getX();
		y = myTouch.getY();

		if ((x >= 365) && (x <= 415))
		{
			if ((y >= 100) && (y <= 130))
			{
				waitForItRect(365, 100, 415, 130);
				return 3;
			}
		}
		if ((y >= 180) && (y <= 215))
		{
			if ((x >= 155) && (x <= 275))
			{
				waitForItRect(155, 180, 275, 215);
				return 1;
			}
			if ((x >= 285) && (x <= 405))
			{
				waitForItRect(285, 180, 405, 215);
				return 2;
			}
		}
	}
	return 0;
}

// Button functions for main menu
void menuButtons()
{
	// Touch screen controls
	if (myTouch.dataAvailable())
	{
		myTouch.read();
		x = myTouch.getX();
		y = myTouch.getY();

		if ((x >= 5) && (x <= 125))
		{
			if ((y >= 32) && (y <= 83))
			{
				// CANBUS
				waitForItMenu(5, 32, 125, 83);
				page = 0;
				hasDrawn = false;
				graphicLoaderState = 0;
			}
			if ((y >= 88) && (y <= 140))
			{
				// VEHTOOL
				waitForItMenu(5, 88, 125, 140);
				page = 9;
				hasDrawn = false;
				graphicLoaderState = 0;
			}
			if ((y >= 145) && (y <= 197))
			{
				// RZRTOOL
				waitForItMenu(5, 145, 125, 197);
				page = 18;
				hasDrawn = false;
				graphicLoaderState = 0;
			}
			if ((y >= 202) && (y <= 254))
			{
				// EXTRAFN
				waitForItMenu(5, 202, 125, 254);
				page = 27;
				hasDrawn = false;
				graphicLoaderState = 0;
			}
			if ((y >= 259) && (y <= 312))
			{
				// SETTING
				waitForItMenu(5, 259, 125, 312);
				page = 36;
				hasDrawn = false;
				graphicLoaderState = 0;
			}
		}
	}
}

// the setup function runs once when you press reset or power the board
void setup()
{
	Serial.begin(115200);
	SerialUSB.begin(CAN_BPS_500K);

	can1.startCAN0(0x000, 0x800);
	can1.startCAN1(0x000, 0x800);

	(sdCard.startSD()) ? Serial.println("SD Running") : Serial.println("SD failed");

	// Initialize the rtc object
	rtc.begin();
#if defined UPDATE_CLOCK
	rtc.setDOW(FRIDAY);
	rtc.setTime(__TIME__);
	rtc.setDate(__DATE__);
#endif
	// LCD  and touch screen settings
	myGLCD.InitLCD();
	myGLCD.clrScr();

	myTouch.InitTouch();
	myTouch.setPrecision(PREC_MEDIUM);

	myGLCD.setFont(BigFont);
	myGLCD.setBackColor(0, 0, 255);

	drawMenu();

	// Draw the Hypertech logo
	bmpDraw("System/HYPER.bmp", 0, 0);
}

/*=========================================================
	Background Processes
===========================================================*/
// Displays time on menu
void updateTime()
{
	if (millis() - timer2 > 1000)
	{
		char time[40];
		drawRoundBtn(5, 5, 125, 30, rtc.getTimeStr(), menuBackground, menuBackground, menuBtnText, CENTER);
		timer2 = millis();
	}
}

// Sends CAN Bus traffic to serial as a background process
void serialOut()
{
	(isSerialOut) && (can1.SerialOutCAN(selectedChannelOut));
}

// Unselects menu button after touch
void waitForItBackground()
{
	if (isWaitForIt && millis() - waitForItTimer > 50)
	{
		myGLCD.setColor(menuBtnBorder);
		myGLCD.drawRoundRect(x1_, y1_, x2_, y2_);
		isWaitForIt = false;
	}
}

// Calls pageControl with a value of 1 to set view page as the home page
void loop()
{
	// GUI
	pageControl();

	// Background Processes
	updateTime();
	serialOut();
	waitForItBackground();
}
