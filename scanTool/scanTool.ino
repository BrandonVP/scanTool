/*
 ===========================================================================
 Name        : scanTool.ino
 Author      : Brandon Van Pelt
 Created	 : 11/15/2020 8:27:18 AM
 Description : Main
 ===========================================================================
 */

/*=========================================================
	Todo List
===========================================================
Read Vehicle DTCs
Daylight savings option (clock menu in settings)

- WiFi - 
delete dongle confirmation

- Playback - 
Redo GUI to match send
Function menu (split, filter, edit?)

- CANBusCapture
Move SD Card file name to non-blocking state

Add library files directly

Add Wi-Fi baud rate change
Move filter mask to ESP32
===========================================================
	End Todo List
=========================================================*/

// Libraries
#include <malloc.h>
#include <memorysaver.h>
#include <SD.h>
#include <SPI.h>
#include <string.h>
#include <UTouchCD.h>
#include <UTFT.h>
#include <UTouch.h>

// Source
#include "ExtraFunctions.h"
#include "PolarisTools.h"
#include "common.h"
#include "CANBus.h"
#include "definitions.h"
#include "CANBusCapture.h"
#include "SDCard.h"
#include "Settings.h"
#include "VehicleTools.h"
#include "Variables.h"

// Harware Objects
CANBus can1;
SDCard sdCard;
DS3231 rtc(SDA, SCL);

// LCD display
//(byte model, int RS, int WR, int CS, int RST, int SER)
UTFT myGLCD(ILI9488_16, 7, 38, 9, 10);
//RTP: byte tclk, byte tcs, byte din, byte dout, byte irq
UTouch myTouch(2, 6, 3, 4, 5);

// For touch controls
int x, y;

// Used for page control
uint8_t nextPage = 0;
uint8_t page = 0;
bool hasDrawn = false;

// *Used by background process*
uint8_t selectedChannelOut = 0;
uint8_t selectedSourceOut = 0;
uint32_t updateClock = 0;
bool isSerialOut = false;
bool isSDOut = false;
bool isMSGSpam = false;

// General use variables
uint8_t state = 0;

// Filter range / Filter Mask
uint32_t CAN0Filter = 0x000;
uint32_t CAN0Mask = 0x000;
uint32_t CAN1Filter = 0x000;
uint32_t CAN1Mask = 0x000;
uint32_t CANWiFiFilter = 0x000;
uint32_t CANWiFiMask = 0x000;

// Use to load pages in pieces to prevent blocking while loading entire page
uint8_t graphicLoaderState = 0;

/*
Uncomment to update the clock then comment out and upload to
the device a second time to prevent updating time to last time
device was on at every startup.
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

// Simplifies getting x and y coords
bool Touch_getXY()
{
	if (myTouch.dataAvailable())
	{
		myTouch.read();
		x = myTouch.getX();
		y = myTouch.getY();
		return true;
	}
	return false;
}

// Detect swipe guesture
uint8_t swipe(uint32_t& lastTouch, uint8_t& initiated, uint16_t& initial_x, uint16_t& initial_y, uint16_t& last_x, uint16_t& last_y)
{
	if (Touch_getXY() == true)
	{
		if (!initiated)
		{
			initial_x = x;
			initial_y = y;
			initiated = true;
		}
		if (x > 0)
		{
			last_x = x;
		}
		if (y > 0)
		{
			last_y = y;
		}
		lastTouch = millis();
	}
	if (initiated == true && millis() - lastTouch > 150)
	{
		if (initial_y < last_y && last_y - initial_y > 90)
		{
			initiated = false;
			initial_x = 0;
			initial_y = 0;
			last_x = 0;
			last_y = 0;
			return SWIPE_DOWN;
		}
		else if (initial_y > last_y && initial_y - last_y > 90)
		{
			initiated = false;
			initial_x = 0;
			initial_y = 0;
			last_x = 0;
			last_y = 0;
			return SWIPE_UP;
		}
		if (initial_x < last_x && last_x - initial_x > 80)
		{
			initiated = false;
			initial_x = 0;
			initial_y = 0;
			last_x = 0;
			last_y = 0;
			return SWIPE_RIGHT;
		}
		else if (initial_x > last_x && initial_x - last_x > 80)
		{
			initiated = false;
			initial_x = 0;
			initial_y = 0;
			last_x = 0;
			last_y = 0;
			return SWIPE_LEFT;
		}
	}
return 0;
}

// Resets variables for page change
void pageTransition()
{
	hasDrawn = false;
	graphicLoaderState = 0;
	page = nextPage;
}

// Manages the different App pages
void pageControl()
{
	switch (page)
	{
	case CANBUS_MAIN: /*========== CANBUS ==========*/
		// Draw page and lock variables
		if (!hasDrawn)
		{
			if (drawCANBus())
			{
				break;
			}
			hasDrawn = true;
		}

		// Call buttons or page method
		CANBusButtons();

		// Release any variable locks if page changed
		if (nextPage != page)
		{
			pageTransition();
		}
		break;

	case 1: // Capture
		// TODO: Fix this hot mess. Need to be in standard format like the rest of the pages.
		// Draw page and lock variables
		switch (state)
		{
		case 0:
			if (!hasDrawn && graphicLoaderState < 1)
			{
				error_t e = false;
				(lockVar16(POS0)) ? g_var16[POS0] = 0 : e = lockError(POS0, 16);
				if (e)
				{
					DEBUG_ERROR(F("Error: Variable locked"));
					delay(5000);
					graphicLoaderState = 0;
					nextPage = CANBUS_MAIN;
				}
			}
			if (!hasDrawn && drawCapture())
			{
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
			if (!hasDrawn && drawCaptureSource())
			{
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
				if (drawCaptureOutput())
				{
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

		// Call buttons or page method
		CaptureButtons();

		// Release any variable locks if page changed
		if (nextPage != page)
		{
			unlockVar16(POS0);
			pageTransition();
		}
		break;

	case 2: // Capture files
		// Draw page and lock variables
		if (!hasDrawn)
		{
			if (drawCANLog())
			{
				break;
			}

			error_t e = false;
			(lockVar32(POS0)) ? g_var32[POS0] = 0 : e = lockError(POS0, 32); // File read index
			(lockVar32(POS1)) ? g_var32[POS1] = 0 : e = lockError(POS1, 32); // Time of last touch
			(lockVar16(POS0)) ? g_var16[POS0] = 0 : e = lockError(POS0, 16); // fileList index
			(lockVar16(POS1)) ? g_var16[POS1] = 60 : e = lockError(POS1, 16); // LCD Print line starts at y coord 60
			(lockVar16(POS2)) ? g_var16[POS2] = 0 : e = lockError(POS2, 16); // Initial x postition
			(lockVar16(POS3)) ? g_var16[POS3] = 0 : e = lockError(POS3, 16); // Initial y postition
			(lockVar16(POS4)) ? g_var16[POS4] = 0 : e = lockError(POS4, 16); // Last x postition
			(lockVar16(POS5)) ? g_var16[POS5] = 0 : e = lockError(POS5, 16); // Last y postition
			(lockVar8(POS0)) ? g_var8[POS0] = 0 : e = lockError(POS0, 8); // Scroll index
			(lockVar8(POS1)) ? g_var8[POS1] = 0 : e = lockError(POS1, 8); // Swipe input direction
			(lockVar8(POS2)) ? g_var8[POS2] = 0 : e = lockError(POS2, 8); // Bool to track initial x/y position
			(lockVar8(POS3)) ? g_var8[POS3] = 0 : e = lockError(POS3, 8); // Bool to track end of file
			if (e)
			{
				DEBUG_ERROR(F("Error: Variable locked"));
				delay(5000);
				graphicLoaderState = 0;
				nextPage = CANBUS_MAIN;
			}

			drawCANLogScroll();
			state = 0;
			hasDrawn = true;
		}

		// Call buttons or page method
		playback();

		// Release any variable locks if page changed
		if (nextPage != page)
		{
			unlockVar8(POS0);
			unlockVar8(POS1);
			unlockVar8(POS2);
			unlockVar8(POS3);
			unlockVar16(POS0);
			unlockVar16(POS1);
			unlockVar16(POS2);
			unlockVar16(POS3);
			unlockVar16(POS4);
			unlockVar16(POS5);
			unlockVar32(POS0);
			unlockVar32(POS1);
			pageTransition();
		}
		break;

	case 3: // Unused
		// Draw page and lock variables
		if (!hasDrawn)
		{
			error_t e = false;

			if (e)
			{
				DEBUG_ERROR(F("Error: Variable locked"));
				nextPage = CANBUS_MAIN;
			}

			hasDrawn = true;
		}

		// Call buttons or page method
		

		// Release any variable locks if page changed
		if (nextPage != page)
		{
			pageTransition();
		}
		break;

	case 4: // Filter Mask
		// Draw page and lock variables
		if (!hasDrawn)
		{
			if (drawFilterMask())
			{
				break;
			}

			error_t e = false;
			(lockVar8(POS0)) ? g_var8[POS0] = 0 : e = lockError(POS0, 8); // Keypad return
			(lockVar8(POS1)) ? g_var8[POS1] = 0 : e = lockError(POS1, 8); // Keypad index
			(lockVar16(POS0)) ? g_var16[POS0] = 0 : e = lockError(POS0, 16); // Current total value
			if (e)
			{
				DEBUG_ERROR(F("Error: Variable locked"));
				delay(5000);
				graphicLoaderState = 0;
				nextPage = CANBUS_MAIN;
			}

			state = 1;
			hasDrawn = true;
		}

		// Call buttons or page method
		filterMask();

		// Release any variable locks if page changed
		if (nextPage != page)
		{
			unlockVar8(POS0);
			unlockVar8(POS1);
			unlockVar16(POS0);
			pageTransition();
		}
		break;

	case 5: // Set Baud
		// Draw page and lock variables
		if (!hasDrawn)
		{
			if (drawBaud())
			{
				break;
			}

			error_t e = false;
			(lockVar8(POS0)) ? g_var8[POS0] = 0 : e = true; // Selected Baud rate array index
			if (e)
			{
				DEBUG_ERROR(F("Error: Variable locked"));
				delay(5000);
				graphicLoaderState = 0;
				nextPage = CANBUS_MAIN;
			}

			drawBaudScroll();
			drawCurrentBaud();

			hasDrawn = true;
		}

		// Call buttons or page method
		baudButtons();

		// Release any variable locks if page changed
		if (nextPage != page)
		{
			unlockVar8(POS0);
			pageTransition();
		}
		break;

	case 6: // Timed TX
		// Draw page and lock variables
		if (!hasDrawn)
		{
			
			if (drawTimedTX())
			{
				break;
			}
			
			error_t e = false;
			(lockVar8(POS0)) ? g_var8[POS0] = 0 : e = lockError(POS0, 8);    // Selected index
			(lockVar8(POS1)) ? g_var8[POS1] = 0 : e = lockError(POS1, 8);    // User input
			(lockVar8(POS2)) ? g_var8[POS2] = 0 : e = lockError(POS2, 8);    // Keypad index
			(lockVar8(POS3)) ? g_var8[POS3] = 0 : e = lockError(POS3, 8);    // Scroll index
			(lockVar8(POS4)) ? g_var8[POS4] = 0 : e = lockError(POS4, 8);    // Node position
			(lockVar8(POS5)) ? g_var8[POS5] = 0 : e = lockError(POS5, 8);    // Keyboard index
			(lockVar8(POS6)) ? g_var8[POS6] = 0 : e = lockError(POS6, 8);    // Delete node location
			(lockVar16(POS0)) ? g_var16[POS0] = 0 : e = lockError(POS0, 16); // Total value
			(lockVar32(POS0)) ? g_var32[POS0] = 0 : e = lockError(POS0, 32); // Timer to prevent double tap
			if (e)
			{
				DEBUG_ERROR(F("Error: Variable locked"));
				delay(5000);
				graphicLoaderState = 0;
				nextPage = CANBUS_MAIN;
			}

			drawTXNode(g_var8[POS3]);

			state = 0;
			hasDrawn = true;
		}
		// Call buttons or page method
		timedTX();

		// Release any variable locks if page changed
		if (nextPage != page)
		{
			unlockVar32(POS0);
			unlockVar16(POS0);
			unlockVar8(POS0);
			unlockVar8(POS1);
			unlockVar8(POS2);
			unlockVar8(POS3);
			unlockVar8(POS4);
			unlockVar8(POS5);
			unlockVar8(POS6);
			pageTransition();
		}
		break;

	case 7: // LCD Capture
		// Draw page and lock variables
		if (!hasDrawn)
		{
			drawReadInCANLCD();

			error_t e = false;
			(lockVar32(POS1)) ? g_var32[POS1] = 0 : e = lockError(POS1, 32);  // Time of last touch
			(lockVar16(POS0)) ? g_var16[POS0] = 60 : e = lockError(POS0, 16); // LCD Print line starts at y coord 60
			(lockVar16(POS1)) ? g_var16[POS1] = 0 : e = lockError(POS1, 16);  // Initial x postition
			(lockVar16(POS2)) ? g_var16[POS2] = 0 : e = lockError(POS2, 16);  // Initial y postition
			(lockVar16(POS3)) ? g_var16[POS3] = 0 : e = lockError(POS3, 16);  // Last x postition
			(lockVar16(POS4)) ? g_var16[POS4] = 0 : e = lockError(POS4, 16);  // Last y postition
			(lockVar8(POS0)) ? g_var8[POS0] = 0 : e = lockError(POS0, 8);     // Swipe input direction
			(lockVar8(POS1)) ? g_var8[POS1] = 0 : e = lockError(POS1, 8);     // Bool to track initial x/y position
			if (e)
			{
				DEBUG_ERROR(F("Error: Variable locked"));
				delay(5000);
				graphicLoaderState = 0;
				nextPage = CANBUS_MAIN;
			}

			isSerialOut = false;
			hasDrawn = true;
		}

		// Call buttons or page method
		g_var8[POS0] = swipe(g_var32[POS1], g_var8[POS1], g_var16[POS1], g_var16[POS2], g_var16[POS3], g_var16[POS4]);
		if ((g_var8[POS0] == SWIPE_DOWN || g_var8[POS0] == SWIPE_UP) && !Touch_getXY())
		{
			g_var16[POS0] = 60;
			drawReadInCANLCD();
		}
		if (g_var8[POS0] == SWIPE_RIGHT && !Touch_getXY())
		{
			state = 0;
			nextPage = 1;
			graphicLoaderState = 0;
		}

		if (!Touch_getXY())
		{
			readInCANMsg(selectedChannelOut);
		}
		// Release any variable locks if page changed
		if (nextPage != page)
		{
			unlockVar8(POS0);
			unlockVar8(POS1);
			unlockVar16(POS0);
			unlockVar16(POS1);
			unlockVar16(POS2);
			unlockVar16(POS3);
			unlockVar16(POS4);
			unlockVar32(POS1);
			pageTransition();
		}
		break;
   
	case 8: // Unused
		// Draw page and lock variables
		if (!hasDrawn)
		{
			hasDrawn = true;
		}

		// Call buttons or page method


		// Release any variable locks if page changed
		if (nextPage != page)
		{
			pageTransition();
		}
		break;

	case VEHTOOL_MAIN: /*========== VEHTOOL ==========*/
		// Draw page and lock variables
		if (!hasDrawn)
		{
			if (drawVehicleTools())
			{
				break;
			}

			hasDrawn = true;
		}

		// Call buttons or page method
		VehicleToolButtons();

		// Release any variable locks if page changed
		if (nextPage != page)
		{
			pageTransition();
		}
		break;

	case 10: // PIDSCAN
		// Draw page and lock variables
		if (!hasDrawn)
		{
			if (drawPIDSCAN())
			{
				break;
			}

			// Lock global variables
			error_t e = false;
			(lockVar8(POS0)) ? g_var8[POS0] = 0 : e = lockError(POS0, 8);	 // VIN request state
			(lockVar8(POS1)) ? g_var8[POS1] = 0 : e = lockError(POS1, 8);    // Loading bar index
			(lockVar8(POS2)) ? g_var8[POS2] = 1 : e = lockError(POS2, 8);    // PIDSCAN state
			(lockVar16(POS0)) ? g_var16[POS0] = 0 : e = lockError(POS0, 16); // PID request range
			(lockVar16(POS1)) ? g_var16[POS1] = 0 : e = lockError(POS1, 16); // PID request bank 
			if (e)
			{
				DEBUG_ERROR(F("Error: Variable locked"));
				delay(5000);
				graphicLoaderState = 0;
				nextPage = VEHTOOL_MAIN;
			}

			// Filter out some unwanted traffic
			can1.setFilterMask0(0x7E0, 0x7D0);

			// Draw the load bar
			loadBar(g_var8[POS1]++);

			state = 0;
			hasDrawn = true;
		}

		// Call buttons or page method
		startPIDSCAN();

		// Release any variable locks if page changed
		if (nextPage != page)
		{
			can1.setFilterMask0(0x000, 0x000);
			unlockVar8(POS0);
			unlockVar8(POS1);
			unlockVar8(POS2);
			unlockVar16(POS0);
			unlockVar16(POS1);
			pageTransition();
		}
		break;

	case 11: // PIDSTRM
		// Draw page and lock variables
		if (!hasDrawn)
		{
			if (hasPID == true)
			{
				if (drawPIDStream())
				{
					break;
				}

				error_t e = false;
				(lockVar8(POS0)) ? g_var8[POS0] = 0 : e = lockError(POS0, 8); // Draw scroll index
				(lockVar16(POS0)) ? g_var16[POS0] = 0 : e = lockError(POS0, 16); // Selected PID to request
				(lockVar16(POS1)) ? g_var16[POS1] = 0 : e = lockError(POS1, 16); // PID samples counter
				(lockVar32(POS0)) ? g_var32[POS0] = 0 : e = lockError(POS0, 32); // PID request timer
				(lockVar32(POS1)) ? g_var32[POS1] = 0 : e = lockError(POS1, 32); // No PIDSCAN timer
				if (e)
				{
					DEBUG_ERROR(F("Error: Variable locked"));
					delay(5000);
					graphicLoaderState = 0;
					nextPage = VEHTOOL_MAIN;
				}

				// Filter out some unwanted traffic
				can1.setFilterMask0(0x7E0, 0x1F0);

				// Read in the PID scan results
				for (int i = 0; i < 80; i++)
				{
					arrayIn[i] = 0x00;
				}
				sdCard.readFile(can1.getFullDir(), arrayIn);

				// Draw the read in results to screen
				drawPIDStreamScroll();
			}
			else
			{
				drawErrorMSG2(F("Error"), F("Please Perform"), F("PIDSCAN First"));
				g_var32[POS1] = millis();
			}

			state = 0;
			hasDrawn = true;
		}

		if(hasPID)
		{
			// Call buttons or page method
			streamPIDS();
		}
		else // Error message buttons
		{
			if (Touch_getXY())
			{
				if ((x >= 365) && (x <= 401))
				{
					if ((y >= 100) && (y <= 130))
					{
						nextPage = VEHTOOL_MAIN;
						break; // Added break to create a small delay preventing double tap
					}
				}
			}
			if (millis() - g_var32[POS1] > 10000)
			{
				nextPage = VEHTOOL_MAIN;
			}
		}
	
		// Release any variable locks if page changed
		if (nextPage != page)
		{
			can1.setFilterMask0(0x0, 0x0);
			unlockVar8(POS0);
			unlockVar16(POS0);
			unlockVar16(POS1);
			unlockVar32(POS0);
			pageTransition();
		}
		break;

	case 12: // PID Guages
		// Draw page and lock variables
		if (!hasDrawn)
		{
			drawPIDGauges();
			hasDrawn = true;
			//can1.startPID();
		}

		// Call buttons or page method
		PIDGauges();

		// Release any variable locks if page changed
		if (nextPage != page)
		{
			pageTransition();
		}
		break;

	case 13: // VIN
		// Draw page and lock variables
		if (!hasDrawn)
		{
			if (state < 4 && !can1.VINReady())
			{
				isSerialOut = false;
				state = can1.requestVIN(state, false);
			}
			else
			{
				hasDrawn = true;
				drawVIN();
			}
		}

		// Call buttons or page method

		// Release any variable locks if page changed
		if (nextPage != page)
		{
			can1.setFilterMask0(0x0, 0x0);
			pageTransition();
		}
		break;

	case 14: // 
		// Draw page and lock variables
		if (!hasDrawn)
		{

			hasDrawn = true;
		}

		// Call buttons or page method

		// Release any variable locks if page changed
		if (nextPage != page)
		{
			pageTransition();
		}
		break;

	case 15: // DTC
		// Draw page and lock variables
		if (!hasDrawn)
		{
			if (drawDTC())
			{
				break;
			}
			// Lock global variables
			error_t e = false;
			(lockVar8(POS0)) ? g_var8[POS0] = 0 : e = lockError(POS0, 8); // Clear DTC state
			(lockVar16(POS1)) ? g_var16[POS1] = 0 : e = lockError(POS1, 16); // Loadbar divider
			(lockVar32(POS0)) ? g_var32[POS0] = 0 : e = lockError(POS0, 32); // Send clear message timer
			if (e)
			{
				DEBUG_ERROR(F("Error: Variable locked"));
				delay(5000);
				graphicLoaderState = 0;
				nextPage = VEHTOOL_MAIN;
			}

			// Initialize state machine variables to 0
			hasDrawn = true;
			state = 0;
		}

		// Call buttons or page method
		DTC();

		// Release any variable locks if page changed
		if (nextPage != page)
		{
			unlockVar8(POS0);
			unlockVar16(POS1);
			unlockVar32(POS0);
			pageTransition();
		}
		break;

	case 16:
		// Draw page and lock variables
		if (!hasDrawn)
		{
			hasDrawn = true;
		}

		// Call buttons or page method

		// Release any variable locks if page changed
		if (nextPage != page)
		{
			pageTransition();
		}
		break;

	case 17:
		// Draw page and lock variables
		if (!hasDrawn)
		{
			hasDrawn = true;
		}
		// Call buttons or page method

		// Release any variable locks if page changed
		if (nextPage != page)
		{
			pageTransition();
		}
		break;

	case UTVTOOL_MAIN: /*========== UTV TOOLS ==========*/
		// Draw page and lock variables
		if (!hasDrawn)
		{
			if (drawRZRTOOL())
			{
				break;
			}

			hasDrawn = true;
		}

		// Call buttons or page method
		RZRToolButtons();

		// Release any variable locks if page changed
		if (nextPage != page)
		{
			pageTransition();
		}
		break;

	case 19: //
		// Draw page and lock variables
		if (!hasDrawn)
		{
			hasDrawn = true;
		}

		// Call buttons or page method

		// Release any variable locks if page changed
		if (nextPage != page)
		{
			pageTransition();
		}
		break;

	case 20: //
		// Draw page and lock variables
		if (!hasDrawn)
		{
			hasDrawn = true;
		}

		// Call buttons or page method

		// Release any variable locks if page changed
		if (nextPage != page)
		{
			pageTransition();
		}
		break;

	case 21: //
		// Draw page and lock variables
		if (!hasDrawn)
		{
			hasDrawn = true;
		}

		// Call buttons or page method

		// Release any variable locks if page changed
		if (nextPage != page)
		{
			pageTransition();
		}
		break;

	case 22: //
		// Draw page and lock variables
		if (!hasDrawn)
		{
			hasDrawn = true;
		}

		// Call buttons or page method

		// Release any variable locks if page changed
		if (nextPage != page)
		{
			pageTransition();
		}
		break;

	case 23: //
		// Draw page and lock variables
		if (!hasDrawn)
		{
			hasDrawn = true;
		}

		// Call buttons or page method

		// Release any variable locks if page changed
		if (nextPage != page)
		{
			pageTransition();
		}
		break;

	case 24: //
		// Draw page and lock variables
		if (!hasDrawn)
		{
			hasDrawn = true;
		}

		// Call buttons or page method

		// Release any variable locks if page changed
		if (nextPage != page)
		{
			pageTransition();
		}
		break;

	case 25: //
		// Draw page and lock variables
		if (!hasDrawn)
		{
			hasDrawn = true;
		}

		// Call buttons or page method

		// Release any variable locks if page changed
		if (nextPage != page)
		{
			pageTransition();
		}
		break;

	case 26: //
		// Draw page and lock variables
		if (!hasDrawn)
		{
			hasDrawn = true;
		}

		// Call buttons or page method

		// Release any variable locks if page changed
		if (nextPage != page)
		{
			pageTransition();
		}
		break;

	case TESTING_MAIN: /*========== EXTRAFN ==========*/
		// Draw page and lock variables
		if (!hasDrawn)
		{
			if (drawExtraFN())
			{
				break;
			}

			hasDrawn = true;
		}

		// Call buttons or page method
		extraFNButtons();

		// Release any variable locks if page changed
		if (nextPage != page)
		{
			pageTransition();
		}
		break;

	case 28: // OBD CAN Simulator
		// Draw page and lock variables
		if (!hasDrawn)
		{
			drawOBDSimulator();
			hasDrawn = true;
		}

		// Call buttons or page method
		entry();

		// Release any variable locks if page changed
		if (nextPage != page)
		{
			pageTransition();
		}
		break;

	case 29: // Message Spam
		// Draw page and lock variables
		// TODO: Clean up this mess / Move to testing
		if (!hasDrawn)
		{
			if (graphicLoaderState == 1)
			{
				// Lock global variables
				error_t e = false;
				(lockVar8(POS1)) ? g_var8[POS1] = 0 : e = lockError(POS1, 8);        // Keypad index
				(lockVar16(POS0)) ? g_var16[POS0] = 0 : e = lockError(POS0, 16);     // ID
				(lockVar16(POS1)) ? g_var16[POS1] = 0x7FF : e = lockError(POS1, 16); // Max
				(lockVar16(POS2)) ? g_var16[POS2] = 0x000 : e = lockError(POS2, 16); // Min
				(lockVar16(POS3)) ? g_var16[POS3] = 30 : e = lockError(POS3, 16);    // Interval (ms)
				(lockVar16(POS4)) ? g_var16[POS4] = 30 : e = lockError(POS4, 16);    // Keypad total
				(lockVar32(POS0)) ? g_var32[POS0] = 0 : e = lockError(POS0, 32);     // Timer
				if (e)
				{
					DEBUG_ERROR(F("Error: Variable locked"));
					delay(5000);
					graphicLoaderState = 0;
					nextPage = TESTING_MAIN;
				}
			}
			
			switch (graphicLoaderState)
			{
			case 0:
				break;
			case 1:
				// Above ^^
				break;
			case 2:
				drawMSGSpam();
				break;
			case 3:
				drawMSGSpamMin();
				break;
			case 4:
				drawMSGSpamMax();
				break;
			case 5:
				drawMSGInterval();
				break;
			case 6:
				hasDrawn = true;
				break;
			}
			graphicLoaderState++;
			return;
		}

		// Call buttons or page method
		if (state == 0)
		{
			MSGSpam();
			sendMSGButtons();
			if (state == 1)
			{
				drawKeypadDec();
				g_var8[POS1] = 0;
				g_var16[POS4] = 0;
				keypadInput[0] = 0;
				keypadInput[1] = 0;
				keypadInput[2] = 0;
			}
			if (state == 2 || state == 3)
			{
				drawKeypad();
				g_var8[POS1] = 0;
				g_var16[POS4] = 0;
				keypadInput[0] = 0;
				keypadInput[1] = 0;
				keypadInput[2] = 0;
			}
		}
		if (state == 1)
		{
			MSGSpam();
			uint8_t input = keypadControllerDec(g_var8[POS1], g_var16[POS4]);
			if (input == 0xF1)
			{
				const uint8_t RUN_TIME_OFFSET_MS = 1; // Approximate time it takes send out a single messagein ms
				g_var16[POS3] = g_var16[POS4] - RUN_TIME_OFFSET_MS;
				state = 0;
				hasDrawn = false;
				graphicLoaderState = 2;
			}
			if (input == 0xF0)
			{
				state = 0;
				hasDrawn = false;
				graphicLoaderState = 2;
			}
		}
		if (state == 2)
		{
			MSGSpam();
			uint8_t input = keypadController(g_var8[POS1], g_var16[POS4]);
			if (input == 0xF1)
			{
				g_var16[POS2] = g_var16[POS4];
				state = 0;
				hasDrawn = false;
				graphicLoaderState = 2;
			}
			if (input == 0xF0)
			{
				state = 0;
				hasDrawn = false;
				graphicLoaderState = 2;
			}
		}
		if (state == 3)
		{
			MSGSpam();
			uint8_t input = keypadController(g_var8[POS1], g_var16[POS4]);
			if (input == 0xF1)
			{
				g_var16[POS1] = g_var16[POS4];
				state = 0;
				hasDrawn = false;
				graphicLoaderState = 2;
			}
			if (input == 0xF0)
			{
				state = 0;
				hasDrawn = false;
				graphicLoaderState = 2;
			}
		}

		// Release any variable locks if page changed
		if (nextPage != page)
		{
			unlockVar8(POS1);
			unlockVar16(POS0);
			unlockVar16(POS1);
			unlockVar16(POS2);
			unlockVar16(POS3);
			unlockVar16(POS4);
			unlockVar32(POS0);
			pageTransition();
		}
		break;

	case 30: // Unused
		// Draw page and lock variables
		if (!hasDrawn)
		{
			hasDrawn = true;
		}

		// Call buttons or page method

		// Release any variable locks if page changed
		if (nextPage != page)
		{
			pageTransition();
		}
		break;

	case 31: // Unused
		// Draw page and lock variables
		if (!hasDrawn)
		{
			hasDrawn = true;
		}

		// Call buttons or page method

		// Release any variable locks if page changed
		if (nextPage != page)
		{
			pageTransition();
		}
		break;

	case 32: // Unused
		// Draw page and lock variables
		if (!hasDrawn)
		{
			hasDrawn = true;
		}

		// Call buttons or page method

		// Release any variable locks if page changed
		if (nextPage != page)
		{
			pageTransition();
		}
		break;

	case 33: //
		// Draw page and lock variables
		if (!hasDrawn)
		{
			isSerialOut = true;
			selectedChannelOut = 6;
			hasDrawn = true;
		}

		// Call buttons or page method

		// Release any variable locks if page changed
		if (nextPage != page)
		{
			pageTransition();
		}
		break;

	case 34: // Ford Dongle
		// Draw page and lock variables
		if (!hasDrawn)
		{
			if (drawDongleSimFord())
			{
				break;
			}

			error_t e = false;
			(lockVar32(POS0)) ? g_var32[POS0] = 0 : e = lockError(POS0, 32);
			if (e)
			{
				DEBUG_ERROR(F("Error: Variable locked"));
				delay(5000);
				graphicLoaderState = 0;
				nextPage = TESTING_MAIN;
			}
			hasDrawn = true;
		}
		
		// Call buttons or page method
		dongleSimButtonsFord();

		// Release any variable locks if page changed
		if (nextPage != page)
		{
			unlockVar32(POS0);
			pageTransition();
		}
		break;

	case 35: // GM Dongle
		// Draw page and lock variables
		if (!hasDrawn)
		{
			if (drawDongleSim())
			{
				break;
			}

			error_t e = false;
			(lockVar32(POS0)) ? g_var32[POS0] = 0 : e = lockError(POS0, 32);
			if (e)
			{
				DEBUG_ERROR(F("Error: Variable locked"));
				delay(5000);
				graphicLoaderState = 0;
				nextPage = TESTING_MAIN;
			}

			hasDrawn = true;
		}

		// Call buttons or page method
		dongleSimButtons();

		// Release any variable locks if page changed
		if (nextPage != page)
		{
			unlockVar32(POS0);
			pageTransition();
		}
		break;

	case SETTING_MAIN: /*========== SETTINGS ==========*/
		// Draw page and lock variables
		if (!hasDrawn)
		{
			if (drawSettings())
			{
				break;
			}

			hasDrawn = true;
		}

		// Call buttons or page method
		settingsButtons();

		// Release any variable locks if page changed
		if (nextPage != page)
		{
			pageTransition();
		}
		break;

	case 37: // Memory
		// Draw page and lock variables
		if (!hasDrawn)
		{
			memoryUse();
			hasDrawn = true;
		}

		// Call buttons or page method

		// Release any variable locks if page changed
		if (nextPage != page)
		{
			pageTransition();
		}
		break;

	case 38: // About
		// Draw page and lock variables
		if (!hasDrawn)
		{
			if (drawAbout())
			{
				break;
			}
			hasDrawn = true;
		}
		// Call buttons or page method

		// Release any variable locks if page changed
		if (nextPage != page)
		{
			pageTransition();
		}
		break;

	case 39: // WiFi MAC Address
		// Draw Page
		// TODO: make a non-blocking state machine
		// TODO: Organize Serial3 commands
		if (!hasDrawn)
		{
			if (isSerialOut && selectedChannelOut == 6)
			{
				// Cant get mac while streaming CAN Bus data over WiFi
				// Can be fixed by printing all at once instead
				hasDrawn = true;
				break;
			}

			// Send command
			Serial3.write(0xAC);
			delay(1);
			Serial3.write(0xAD);
			delay(1);
			if (drawMACAddress())
			{
				break;
			}
			hasDrawn = true;
		}

		// Call buttons or page method

		// Release any variable locks if page changed
		if (nextPage != page)
		{
			pageTransition();
		}
		break;

	case 40: // Unused
		// Draw page and lock variables
		if (!hasDrawn)
		{
			hasDrawn = true;
		}

		// Call buttons or page method

		// Release any variable locks if page changed
		if (nextPage != page)
		{
			pageTransition();
		}
		break;

	case 41: // Unused
		// Draw page and lock variables
		if (!hasDrawn)
		{
			hasDrawn = true;
		}

		// Call buttons or page method

		// Release any variable locks if page changed
		if (nextPage != page)
		{
			pageTransition();
		}
		break;

	case 42: // Unused
		// Draw page and lock variables
		if (!hasDrawn)
		{
			hasDrawn = true;
		}

		// Call buttons or page method

		// Release any variable locks if page changed
		if (nextPage != page)
		{
			pageTransition();
		}
		break;

	case 43: // Connect Dongle
		// Draw page and lock variables
		if (!hasDrawn)
		{
			error_t e = false;
			(lockVar8(POS0)) ? g_var8[POS0] = 0 : e = lockError(POS0, 8);    // Selected index
			(lockVar8(POS1)) ? g_var8[POS1] = 0 : e = lockError(POS1, 8);    // User input
			(lockVar8(POS2)) ? g_var8[POS2] = 0 : e = lockError(POS2, 8);    // Keypad index
			(lockVar8(POS3)) ? g_var8[POS3] = 0 : e = lockError(POS3, 8);    // Scroll index
			(lockVar8(POS4)) ? g_var8[POS4] = 0 : e = lockError(POS4, 8);    // Node position
			(lockVar8(POS5)) ? g_var8[POS5] = 0 : e = lockError(POS5, 8);    // Keyboard index
			(lockVar16(POS0)) ? g_var16[POS0] = 0 : e = lockError(POS0, 16); // Total value
			if (e)
			{
				DEBUG_ERROR(F("Error: Variable locked"));
				delay(5000);
				graphicLoaderState = 0;
				nextPage = SETTING_MAIN;
			}
			state = 0;
			hasDrawn = true;
		}
		// Call buttons or page method
		connectDongle();

		// Release any variable locks if page changed
		if (nextPage != page)
		{
			unlockVar8(POS0);
			unlockVar8(POS1);
			unlockVar8(POS2);
			unlockVar8(POS3);
			unlockVar8(POS4);
			unlockVar8(POS5);
			unlockVar16(POS0);
			pageTransition();
		}
		break;

	case 44: // Unused
		// Draw page and lock variables
		if (!hasDrawn)
		{
			if (drawClockSpeed())
			{
				break;
			}
			hasDrawn = true;
		}

		// Call buttons or page method
		clockSpeedButtons();

		// Release any variable locks if page changed
		if (nextPage != page)
		{
			pageTransition();
		}
		break;
	}
}

// The setup function runs once when the board is reset or power up
void setup()
{
	DueOverclock.setCoreFrequency(MCUClockSpeed);

	Serial.begin(115200);
	Serial3.begin(115200);
	SerialUSB.begin(115200);

	can1.setBaud0(CAN_BPS_500K);
	can1.setBaud1(CAN_BPS_500K);

	can1.startCAN0(0x000, 0x7FF);
	can1.startCAN1(0x000, 0x7FF);

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

	// Only drawn at startup
	drawMenu();

	// Draw the ScanTool logo
	bmpDraw("System/HYPER.bmp", 0, 0);
	loadRXMsg();
	loadMACs();
}


/*=========================================================
	Buttons / Error Message
===========================================================*/
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

// Holds button down while pressed
void waitForIt(int x1, int y1, int x2, int y2)
{
	myGLCD.setColor(themeBackground);
	myGLCD.drawRoundRect(x1, y1, x2, y2);
	while (myTouch.dataAvailable())
	{
		myTouch.read();
		backgroundProcess();
	}

	myGLCD.setColor(menuBtnBorder);
	myGLCD.drawRoundRect(x1, y1, x2, y2);
}
void waitForItRect(int x1, int y1, int x2, int y2)
{
	myGLCD.setColor(themeBackground);
	myGLCD.drawRect(x1, y1, x2, y2);
	while (myTouch.dataAvailable())
	{
		myTouch.read();
		backgroundProcess();
	}
	myGLCD.setColor(menuBtnBorder);
	myGLCD.drawRect(x1, y1, x2, y2);
}

// Function complete status load bar
bool loadBar(int progress)
{
	if (progress >= DONE)
	{
		drawSquareBtn(200, 200, 400, 220, F("Finished"), menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
		//myGLCD.setColor(menuBtnColor);
		//myGLCD.fillRect(200, 200, 400, 220);

		return true;
	}
	drawSquareBtn(199, 199, 401, 221, "", themeBackground, menuBtnBorder, menuBtnText, LEFT);
	myGLCD.setColor(menuBtnColor);
	myGLCD.fillRect(200, 200, (200 + (progress * 25)), 220);
	return false;
}

// Only called once at startup to draw the menu
void drawMenu()
{
	// Draw Layout
	drawSquareBtn(0, 0, 479, 319, "", themeBackground, themeBackground, themeBackground, CENTER);
	drawSquareBtn(0, 0, 130, 319, "", menuBackground, menuBackground, menuBackground, CENTER);

	// Draw Menu Buttons
	drawRoundBtn(5, 32, 125, 83, F("CANBUS"), menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
	drawRoundBtn(5, 88, 125, 140, F("VEHTOOL"), menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
	drawRoundBtn(5, 145, 125, 197, F("UTVTOOL"), menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
	drawRoundBtn(5, 202, 125, 254, F("TESTING"), menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
	drawRoundBtn(5, 259, 125, 312, F("SETTING"), menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
}

/*============== Error Message ==============*/
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

// Notification message
void drawErrorMSG2(String title, String eMessage1, String eMessage2)
{
	drawSquareBtn(145, 100, 401, 220, "", menuBackground, menuBtnColor, menuBtnColor, CENTER);
	drawSquareBtn(145, 100, 401, 130, title, themeBackground, menuBtnColor, menuBtnBorder, LEFT);
	drawSquareBtn(146, 131, 400, 155, eMessage1, menuBackground, menuBackground, menuBtnText, CENTER);
	drawSquareBtn(146, 155, 400, 180, eMessage2, menuBackground, menuBackground, menuBtnText, CENTER);
	drawRoundBtn(365, 100, 401, 130, "X", menuBtnColor, menuBtnColor, menuBtnText, CENTER);
}

// Error Message buttons, returns 0 by default
uint8_t errorMSGButton(uint8_t confirmPage, uint8_t cancelPage, uint8_t xPage)
{
	// Touch screen controls
	if (Touch_getXY())
	{
		if ((x >= 365) && (x <= 415))
		{
			if ((y >= 100) && (y <= 130))
			{
				// X
				waitForItRect(365, 100, 415, 130);
				return xPage;
			}
		}
		if ((y >= 180) && (y <= 215))
		{
			if ((x >= 155) && (x <= 275))
			{
				// Confirm
				waitForItRect(155, 180, 275, 215);
				return confirmPage;
			}
			if ((x >= 285) && (x <= 405))
			{
				// Cancel
				waitForItRect(285, 180, 405, 215);
				return cancelPage;
			}
		}
	}
	return 0;
}


/*=========================================================
	Background Processes
===========================================================*/
// Button functions for the main menu
void menuButtons()
{
	// Touch screen controls
	if (Touch_getXY())
	{
		if ((x >= 5) && (x <= 125))
		{
			if ((y >= 32) && (y <= 83))
			{
				// CANBUS
				waitForIt(5, 32, 125, 83);
				nextPage = 0;
				graphicLoaderState = 0;
			}
			if ((y >= 88) && (y <= 140))
			{
				// VEHTOOL
				waitForIt(5, 88, 125, 140);
				nextPage = 9;
				graphicLoaderState = 0;
			}
			if ((y >= 145) && (y <= 197))
			{
				// RZRTOOL
				waitForIt(5, 145, 125, 197);
				nextPage = 18;
				graphicLoaderState = 0;
			}
			if ((y >= 202) && (y <= 254))
			{
				// EXTRAFN
				waitForIt(5, 202, 125, 254);
				nextPage = 27;
				graphicLoaderState = 0;
			}
			if ((y >= 259) && (y <= 312))
			{
				// SETTING
				waitForIt(5, 259, 125, 312);
				nextPage = 36;
				graphicLoaderState = 0;
			}
		}
	}
}

// Displays time above menu
void updateTime()
{
	const uint16_t SECOND = 1000;

	if (millis() - updateClock > SECOND)
	{
		drawRoundBtn(5, 5, 125, 30, rtc.getTimeStr(), menuBackground, menuBackground, menuBtnText, CENTER);
		updateClock = millis();
	}
}

// Sends CAN Bus traffic to serial as a background process
void serialOut()
{
	(isSerialOut) && (can1.SerialOutCAN(selectedChannelOut));
}

// Sends CAN Bus traffic to serial as a background process
void SDCardOut()
{
	(isSDOut) && (can1.SDOutCAN(selectedChannelOut));
}

// All background process should be called from here
void backgroundProcess()
{
	menuButtons();
	updateTime();
	serialOut();
	SDCardOut();
	timedTXSend();
}


/*=========================================================
	Main loop
===========================================================*/
// Program main loop
void loop()
{
	// GUI
	pageControl();

	// Background Processes
	backgroundProcess();
}
