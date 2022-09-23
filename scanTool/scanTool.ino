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
Daylight savings option (clock menu in settings)

- SD -
Save block when ending capture

- WiFi - 
delete dongle confirmation

- Playback - 
Redo GUI to match send
Function menu (split, filter, edit?)
Views stops working at end of file 

- Memory - 
Reduce usage

===========================================================
	End Todo List
=========================================================*/

//#define DEBUG_KEYBOARD(x)  SerialUSB.println(x);
#define DEBUG_KEYBOARD(x)

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
UTouch  myTouch(2, 6, 3, 4, 5);

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
bool isFinished = false;
uint8_t g_var8[8];
uint8_t g_var8Lock = 0;
uint16_t g_var16[8];
uint8_t g_var16Lock = 0;
uint32_t g_var32[8];
uint8_t g_var32Lock = 0;
char keyboardInput[9];
uint8_t keypadInput[4] = {0, 0, 0, 0};

// Used for converting keypad input to appropriate hex place
const uint32_t hexTable[8] = { 1, 16, 256, 4096, 65536, 1048576, 16777216, 268435456 };

// List of baud rates for Baud page
const uint32_t baudRates[6] = { 1000000, 800000, 500000, 250000, 125000, 100000 };

// TODO: Why are these needed? Can the same function be done using the library?
// Filter range / Filter Mask
uint32_t CAN0Filter = 0x000;
uint32_t CAN0Mask = 0x7FF;
uint32_t CAN1Filter = 0x000;
uint32_t CAN1Mask = 0x7FF;

// Holds CAN Bus capture replay filenames
char fileList[20][13];

// Determines if a PID scan was performed before displaying pid list
bool hasPID = false;

// Holds PIDS for the pidscan function
uint8_t arrayIn[80];

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

// Manages the different App pages
void pageControl()
{
	switch (page)
	{
	case 0: /*========== CANBUS ==========*/
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
			hasDrawn = false;
			page = nextPage;
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
				(lockVar16(LOCK0)) ? g_var16[POS0] = 0 : e = true;
				if (e)
				{
					DEBUG_ERROR(F("Error: Variable locked"));
					nextPage = CANBUS_MAIN;
					break;
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
			unlockVar16(LOCK0);
			hasDrawn = false;
			page = nextPage;
		}
		break;

	case 2: // Playback
		// Draw page and lock variables
		if (!hasDrawn)
		{
			if (drawCANLog())
			{
				break;
			}

			error_t e = false;
			(lockVar32(LOCK0)) ? g_var32[POS0] = 0 : e = true; // File read index
			(lockVar32(LOCK1)) ? g_var32[POS1] = 0 : e = true; // Time of last touch
			(lockVar16(LOCK0)) ? g_var16[POS0] = 0 : e = true; // fileList index
			(lockVar16(LOCK1)) ? g_var16[POS1] = 60 : e = true; // LCD Print line starts at y coord 60
			(lockVar16(LOCK2)) ? g_var16[POS2] = 0 : e = true; // Initial x postition
			(lockVar16(LOCK3)) ? g_var16[POS3] = 0 : e = true; // Initial y postition
			(lockVar16(LOCK4)) ? g_var16[POS4] = 0 : e = true; // Last x postition
			(lockVar16(LOCK5)) ? g_var16[POS5] = 0 : e = true; // Last y postition
			(lockVar8(LOCK0)) ? g_var8[POS0] = 0 : e = true; // Scroll index
			(lockVar8(LOCK1)) ? g_var8[POS1] = 0 : e = true; // Swipe input direction
			(lockVar8(LOCK2)) ? g_var8[POS2] = 0 : e = true; // Bool to track initial x/y position
			if (e)
			{
				DEBUG_ERROR(F("Error: Variable locked"));
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
			unlockVar8(LOCK0);
			unlockVar8(LOCK1);
			unlockVar8(LOCK2);
			unlockVar16(LOCK0);
			unlockVar16(LOCK1);
			unlockVar16(LOCK2);
			unlockVar16(LOCK3);
			unlockVar16(LOCK4);
			unlockVar16(LOCK5);
			unlockVar32(LOCK0);
			unlockVar32(LOCK1);
			hasDrawn = false;
			page = nextPage;
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
			hasDrawn = false;
			page = nextPage;
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
			(lockVar8(LOCK0)) ? g_var8[POS0] = 0 : e = true; // Keypad return
			(lockVar8(LOCK1)) ? g_var8[POS1] = 0 : e = true; // Keypad index
			(lockVar16(LOCK0)) ? g_var16[POS0] = 0 : e = true; // Current total value
			if (e)
			{
				DEBUG_ERROR(F("Error: Variable locked"));
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
			unlockVar8(LOCK0);
			unlockVar8(LOCK1);
			unlockVar16(LOCK0);
			hasDrawn = false;
			page = nextPage;
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
			(lockVar8(LOCK0)) ? g_var8[POS0] = 0 : e = true; // Selected Baud rate array index
			if (e)
			{
				DEBUG_ERROR(F("Error: Variable locked"));
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
			unlockVar8(LOCK0);
			hasDrawn = false;
			page = nextPage;
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
			(lockVar8(LOCK0)) ? g_var8[POS0] = 0 : e = true; // Selected index
			(lockVar8(LOCK1)) ? g_var8[POS1] = 0 : e = true; // User input
			(lockVar8(LOCK2)) ? g_var8[POS2] = 0 : e = true; // Keypad index
			(lockVar8(LOCK3)) ? g_var8[POS3] = 0 : e = true; // Scroll index
			(lockVar8(LOCK4)) ? g_var8[POS4] = 0 : e = true; // Node position
			(lockVar8(LOCK5)) ? g_var8[POS5] = 0 : e = true; // Keyboard index
			(lockVar16(LOCK0)) ? g_var16[POS0] = 0 : e = true; // Total value
			if (e)
			{
				DEBUG_ERROR(F("Error: Variable locked"));
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
			unlockVar16(LOCK0);
			unlockVar8(LOCK0);
			unlockVar8(LOCK1);
			unlockVar8(LOCK2);
			unlockVar8(LOCK3);
			unlockVar8(LOCK4);
			unlockVar8(LOCK5);
			hasDrawn = false;
			page = nextPage;
		}
		break;

	case 7: // LCD Capture
		// Draw page and lock variables
		if (!hasDrawn)
		{
			drawReadInCANLCD();

			error_t e = false;
			(lockVar32(LOCK1)) ? g_var32[POS1] = 0 : e = true; // Time of last touch
			(lockVar16(LOCK0)) ? g_var16[POS0] = 60 : e = true; // LCD Print line starts at y coord 60
			(lockVar16(LOCK1)) ? g_var16[POS1] = 0 : e = true; // Initial x postition
			(lockVar16(LOCK2)) ? g_var16[POS2] = 0 : e = true; // Initial y postition
			(lockVar16(LOCK3)) ? g_var16[POS3] = 0 : e = true; // Last x postition
			(lockVar16(LOCK4)) ? g_var16[POS4] = 0 : e = true; // Last y postition
			(lockVar8(LOCK0)) ? g_var8[POS0] = 0 : e = true; // Swipe input direction
			(lockVar8(LOCK1)) ? g_var8[POS1] = 0 : e = true; // Bool to track initial x/y position
			if (e)
			{
				DEBUG_ERROR(F("Error: Variable locked"));
				nextPage = CANBUS_MAIN;
			}

			isSerialOut = false;
			hasDrawn = true;
		}

		// Call buttons or page method
		g_var8[POS0] = swipe(g_var32[POS1], g_var8[POS1], g_var16[POS1], g_var16[POS2], g_var16[POS3], g_var16[POS4]);

		if (!Touch_getXY())
		{
			readInCANMsg(selectedChannelOut);
		}

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

		// Release any variable locks if page changed
		if (nextPage != page)
		{
			unlockVar8(LOCK0);
			unlockVar8(LOCK1);
			unlockVar16(LOCK0);
			unlockVar16(LOCK1);
			unlockVar16(LOCK2);
			unlockVar16(LOCK3);
			unlockVar16(LOCK4);
			unlockVar32(LOCK0);
			unlockVar32(LOCK1);
			hasDrawn = false;
			page = nextPage;
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
			hasDrawn = false;
			page = nextPage;
		}
		break;

	case 9: /*========== VEHTOOL ==========*/
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
			hasDrawn = false;
			page = nextPage;
		}
		break;

	case 10: // PIDSCAN
		// Draw page and lock variables
		if (!hasDrawn)
		{
			if (hasPID)
			{
				page = VEHTOOL_MAIN;
				break;
			}

			if (drawPIDSCAN())
			{
				break;
			}

			// Lock global variables
			error_t e = false;
			(lockVar8(LOCK0)) ? g_var8[POS0] = 0 : e = true;
			(lockVar8(LOCK1)) ? g_var8[POS1] = 0 : e = true; // Loading bar index
			(lockVar16(LOCK0)) ? g_var16[POS0] = 0 : e = true;
			(lockVar16(LOCK1)) ? g_var16[POS1] = 0 : e = true;
			(lockVar32(LOCK0)) ? g_var32[POS0] = 0 : e = true;
			if (e)
			{
				DEBUG_ERROR(F("Error: Variable locked"));
				nextPage = VEHTOOL_MAIN;
			}

			loadBar(g_var8[POS1]++);
			state = 0;
			isSerialOut = false;
			hasDrawn = true;
		}

		// Call buttons or page method
		startPIDSCAN();

		// Release any variable locks if page changed
		if (nextPage != page)
		{
			unlockVar8(LOCK0);
			unlockVar8(LOCK1);
			unlockVar16(LOCK0);
			unlockVar16(LOCK1);
			unlockVar32(LOCK0);
			hasDrawn = false;
			page = nextPage;
		}
		break;

	case 11: // PIDSTRM
		// Draw page and lock variables
		if (!hasDrawn)
		{
			if (hasPID == true)
			{
				drawPIDStream();

				error_t e = false;
				(lockVar8(LOCK0)) ? g_var8[POS0] = 0 : e = true;
				(lockVar16(LOCK0)) ? g_var16[POS0] = 0 : e = true;
				(lockVar16(LOCK1)) ? g_var16[POS1] = 0 : e = true;
				(lockVar32(LOCK0)) ? g_var32[POS0] = 0 : e = true;
				if (e)
				{
					DEBUG_ERROR(F("Error: Variable locked"));
					nextPage = VEHTOOL_MAIN;
				}

				state = 0;
				isSerialOut = false;
				can1.setFilterMask0(0x7E0, 0x1F0);

				for (int i = 0; i < 80; i++)
				{
					arrayIn[i] = 0x00;
				}
				sdCard.readFile(can1.getFullDir(), arrayIn);

				hasDrawn = true;
			}
			else
			{
				nextPage = VEHTOOL_MAIN;
				drawErrorMSG("Error", "Please Perform", "PIDSCAN First");
				hasDrawn = true;
			}
		}

		// Call buttons or page method
		void streamPIDS();

		// Release any variable locks if page changed
		if (nextPage != page)
		{
			can1.setFilterMask0(0x0, 0x0);
			unlockVar8(LOCK0);
			unlockVar16(LOCK0);
			unlockVar16(LOCK1);
			unlockVar32(LOCK0);
			hasDrawn = false;
			page = nextPage;
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
			hasDrawn = false;
			page = nextPage;
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
			hasDrawn = false;
			page = nextPage;
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
			hasDrawn = false;
			page = nextPage;
		}
		break;

	case 15: // DTC
		// Draw page and lock variables
		if (!hasDrawn)
		{
			if (state == 0)
			{
				drawClearDTC();
				state++;
			}
			if (state == 1)
			{
				state = DTCButtons();
			}
			if (state == 2)
			{
				// Lock global variables
				error_t e = false;
				(lockVar16(LOCK1)) ? g_var16[POS1] = 0 : e = true;
				(lockVar32(LOCK0)) ? g_var32[POS0] = 0 : e = true;
				if (e)
				{
					DEBUG_ERROR(F("Error: Variable locked"));
					nextPage = TESTING_MAIN;
				}

				// Initialize state machine variables to 0
				hasDrawn = true;
				state = 0;
				isFinished = false;
			}
		}

		// Call buttons or page method
		clearDTC();

		// Release any variable locks if page changed
		if (nextPage != page)
		{
			unlockVar16(LOCK1);
			unlockVar32(LOCK0);
			hasDrawn = false;
			page = nextPage;
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
			hasDrawn = false;
			page = nextPage;
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
			hasDrawn = false;
			page = nextPage;
		}
		break;

	case 18: /*========== RZRTOOL ==========*/
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
			hasDrawn = false;
			page = nextPage;
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
			hasDrawn = false;
			page = nextPage;
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
			hasDrawn = false;
			page = nextPage;
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
			hasDrawn = false;
			page = nextPage;
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
			hasDrawn = false;
			page = nextPage;
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
			hasDrawn = false;
			page = nextPage;
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
			hasDrawn = false;
			page = nextPage;
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
			hasDrawn = false;
			page = nextPage;
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
			hasDrawn = false;
			page = nextPage;
		}
		break;

	case 27: /*========== EXTRAFN ==========*/
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
			hasDrawn = false;
			page = nextPage;
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
			hasDrawn = false;
			page = nextPage;
		}
		break;

	case 29: // Message Spam
		// Draw page and lock variables
		// TODO: Clean up this mess
		if (!hasDrawn)
		{
			if (graphicLoaderState == 1)
			{
				// Lock global variables
				error_t e = false;
				(lockVar8(LOCK1)) ? g_var8[POS1] = 0 : e = true; // Keypad index
				(lockVar16(LOCK0)) ? g_var16[POS0] = 0 : e = true; // ID
				(lockVar16(LOCK1)) ? g_var16[POS1] = 0x7FF : e = true; // Max
				(lockVar16(LOCK2)) ? g_var16[POS2] = 0x000 : e = true; // Min
				(lockVar16(LOCK3)) ? g_var16[POS3] = 30 : e = true; // Interval (ms)
				(lockVar16(LOCK4)) ? g_var16[POS4] = 30 : e = true; // Keypad total
				(lockVar32(LOCK0)) ? g_var32[POS0] = 0 : e = true; // Timer
				if (e)
				{
					DEBUG_ERROR(F("Error: Variable locked"));
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
				g_var16[POS3] = g_var16[POS4];
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
			unlockVar8(LOCK1);
			unlockVar16(LOCK0);
			unlockVar16(LOCK1);
			unlockVar16(LOCK2);
			unlockVar16(LOCK3);
			unlockVar16(LOCK4);
			unlockVar32(LOCK0);
			hasDrawn = false;
			page = nextPage;
		}
		break;

	case 30: //
		// Draw page and lock variables
		if (!hasDrawn)
		{
			drawkeyboard();
			hasDrawn = true;
			g_var8[POS0] = 0;
			g_var8[POS1] = 0;
		}

		// Call buttons or page method
		g_var8[POS0] = keyboardController(g_var8[POS1]);
		
		// Release any variable locks if page changed
		if (nextPage != page)
		{
			hasDrawn = false;
			page = nextPage;
		}
		break;

	case 31: //
		// Draw page and lock variables
		if (!hasDrawn)
		{
			// TODO: LOCKS!!!!
			drawKeypad();
			hasDrawn = true;
			g_var8[POS0] = 0;
			g_var8[POS1] = 0;
			g_var16[POS0] = 0;
			keypadInput[0] = 0;
			keypadInput[1] = 0;
			keypadInput[2] = 0;
		}

		g_var8[POS0] = keypadController(g_var8[POS1], g_var16[POS0]);

		// Call buttons or page method

		// Release any variable locks if page changed
		if (nextPage != page)
		{
			hasDrawn = false;
			page = nextPage;
		}
		break;

	case 32: //
		// Draw page and lock variables
		if (!hasDrawn)
		{
			// TODO: Locks!!!!!
			drawKeypadDec();
			hasDrawn = true;
			g_var8[POS0] = 0;
			g_var8[POS1] = 0;
			g_var16[POS0] = 0;
			keypadInput[0] = 0;
			keypadInput[1] = 0;
			keypadInput[2] = 0;
		}

		g_var8[POS0] = keypadControllerDec(g_var8[POS1], g_var16[POS0]);

		// Call buttons or page method

		// Release any variable locks if page changed
		if (nextPage != page)
		{
			hasDrawn = false;
			page = nextPage;
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
			hasDrawn = false;
			page = nextPage;
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
			(lockVar32(LOCK0)) ? g_var32[POS0] = 0 : e = true;
			if (e)
			{
				DEBUG_ERROR(F("Error: Variable locked"));
				nextPage = TESTING_MAIN;
			}
			hasDrawn = true;
		}
		
		// Call buttons or page method
		dongleSimButtonsFord();

		// Release any variable locks if page changed
		if (nextPage != page)
		{
			unlockVar32(LOCK0);
			hasDrawn = false;
			page = nextPage;
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
			(lockVar32(LOCK0)) ? g_var32[POS0] = 0 : e = true;
			if (e)
			{
				DEBUG_ERROR(F("Error: Variable locked"));
				nextPage = TESTING_MAIN;
			}

			hasDrawn = true;
		}

		// Call buttons or page method
		dongleSimButtons();

		// Release any variable locks if page changed
		if (nextPage != page)
		{
			unlockVar32(LOCK0);
			hasDrawn = false;
			page = nextPage;
		}
		break;

	case 36: /*========== SETTINGS ==========*/
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
			hasDrawn = false;
			page = nextPage;
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
			hasDrawn = false;
			page = nextPage;
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
			hasDrawn = false;
			page = nextPage;
		}
		break;

	case 39: //
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
			hasDrawn = false;
			page = nextPage;
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
			hasDrawn = false;
			page = nextPage;
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
			hasDrawn = false;
			page = nextPage;
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
			hasDrawn = false;
			page = nextPage;
		}
		break;

	case 43: // Connect Dongle
		// Draw page and lock variables
		if (!hasDrawn)
		{
			error_t e = false;
			(lockVar8(LOCK0)) ? g_var8[POS0] = 0 : e = true; // Selected index
			(lockVar8(LOCK1)) ? g_var8[POS1] = 0 : e = true; // User input
			(lockVar8(LOCK2)) ? g_var8[POS2] = 0 : e = true; // Keypad index
			(lockVar8(LOCK3)) ? g_var8[POS3] = 0 : e = true; // Scroll index
			(lockVar8(LOCK4)) ? g_var8[POS4] = 0 : e = true; // Node position
			(lockVar8(LOCK5)) ? g_var8[POS5] = 0 : e = true; // Keyboard index
			(lockVar16(LOCK0)) ? g_var16[POS0] = 0 : e = true; // Total value
			if (e)
			{
				DEBUG_ERROR(F("Error: Variable locked"));
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
			unlockVar16(LOCK0);
			unlockVar8(LOCK0);
			unlockVar8(LOCK1);
			unlockVar8(LOCK2);
			unlockVar8(LOCK3);
			unlockVar8(LOCK4);
			unlockVar8(LOCK5);
			hasDrawn = false;
			page = nextPage;
		}
		break;

	case 44: // Unused
		// Draw page and lock variables
		if (!hasDrawn)
		{
			hasDrawn = true;
		}

		// Call buttons or page method

		// Release any variable locks if page changed
		if (nextPage != page)
		{
			hasDrawn = false;
			page = nextPage;
		}
		break;
	}
}

// the setup function runs once when you press reset or power the board
void setup()
{
	Serial.begin(115200);
	Serial3.begin(115200);
	SerialUSB.begin(CAN_BPS_500K);

	can1.setBaud0(500000);
	can1.setBaud1(500000);

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
	Buttons / Keypads
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

// Function complete load bar
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

// Call before using keypad to clear out old values from array used to shift input
void resetKeypad()
{
	for (uint8_t i = 0; i < 4; i++)
	{
	keypadInput[i] = 0;
	}
}

/*============== Hex Keypad ==============*/
// User input keypad
void drawKeypad()
{
	drawSquareBtn(131, 55, 479, 319, "", themeBackground, themeBackground, themeBackground, CENTER);
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
	drawRoundBtn(365, 170, 470, 210, F("<---"), menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
	drawRoundBtn(145, 220, 250, 260, F("Input:"), menuBackground, menuBtnBorder, menuBtnText, CENTER);
	drawRoundBtn(255, 220, 470, 260, F(" "), menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
	drawRoundBtn(145, 270, 305, 310, F("Accept"), menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
	drawRoundBtn(315, 270, 470, 310, F("Cancel"), menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
}

// User input keypad
int keypadButtons()
{
	// Touch screen controls
	if (Touch_getXY())
	{
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
			// Backspace
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

/*
* No change returns 0xFF
* Accept returns 0xF1
* Cancel returns 0xF0
* Value contained in total
*/
uint8_t keypadController(uint8_t& index, uint16_t& total)
{
	uint8_t input = keypadButtons();
	if (input >= 0x00 && input < 0x10 && index < 3)
	{
		keypadInput[2] = keypadInput[1];
		keypadInput[1] = keypadInput[0];
		keypadInput[0] = input;
		total = keypadInput[0] * hexTable[0] + keypadInput[1] * hexTable[1] + keypadInput[2] * hexTable[2];
		drawRoundBtn(255, 220, 470, 260, String(total, 16), menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
		++index;
		return 0xFF;
	}
	if (input == 0x10)
	{
		switch (index)
		{
		case 1:
			keypadInput[0] = 0;
			break;
		case 2:
			keypadInput[0] = keypadInput[1];
			keypadInput[1] = 0;
			break;
		case 3: 
			keypadInput[0] = keypadInput[1];
			keypadInput[1] = keypadInput[2];
			keypadInput[2] = 0;
			break;
		}
		total = keypadInput[0] * hexTable[0] + keypadInput[1] * hexTable[1] + keypadInput[2] * hexTable[2];
		drawRoundBtn(255, 220, 470, 260, String(total, 16), menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
		(index > 0) ? --index : 0;
		return 0xFF;
	}
	if (input == 0x11)
	{
		return 0xF1;
	}
	else if (input == 0x12)
	{
		return 0xF0;
	}
	return input;
}

/*============== Decimal Keypad ==============*/
// User input keypad
void drawKeypadDec()
{
	drawSquareBtn(131, 55, 479, 319, "", themeBackground, themeBackground, themeBackground, CENTER);
	uint16_t posY = 125;
	uint8_t numPad = 0x00;

	for (uint8_t i = 0; i < 2; i++)
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
	drawRoundBtn(365, 170, 470, 210, F("<---"), menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
	drawRoundBtn(145, 220, 250, 260, F("Input:"), menuBackground, menuBtnBorder, menuBtnText, CENTER);
	drawRoundBtn(255, 220, 470, 260, F(" "), menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
	drawRoundBtn(145, 270, 305, 310, F("Accept"), menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
	drawRoundBtn(315, 270, 470, 310, F("Cancel"), menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
}

// User input keypad
int keypadButtonsDec()
{
	// Touch screen controls
	if (Touch_getXY())
	{
		if ((y >= 125) && (y <= 165))
		{
			// 0
			if ((x >= 145) && (x <= 195))
			{
				waitForIt(145, 125, 195, 165);
				return 0x00;
			}
			// 1
			if ((x >= 200) && (x <= 250))
			{
				waitForIt(200, 125, 250, 165);
				return 0x01;
			}
			// 2
			if ((x >= 255) && (x <= 305))
			{
				waitForIt(255, 125, 305, 165);
				return 0x02;
			}
			// 3
			if ((x >= 310) && (x <= 360))
			{
				waitForIt(310, 125, 360, 165);
				return 0x03;
			}
			// 4
			if ((x >= 365) && (x <= 415))
			{
				waitForIt(365, 125, 415, 165);
				return 0x04;
			}
			// 5
			if ((x >= 420) && (x <= 470))
			{
				waitForIt(420, 125, 470, 165);
				return 0x05;
			}
		}
		if ((y >= 170) && (y <= 210))
		{
			// 6
			if ((x >= 145) && (x <= 195))
			{
				waitForIt(145, 170, 195, 210);
				return 0x06;
			}
			// 7
			if ((x >= 200) && (x <= 250))
			{
				waitForIt(200, 170, 250, 210);
				return 0x07;
			}
			// 8
			if ((x >= 255) && (x <= 305))
			{
				waitForIt(255, 170, 305, 210);
				return 0x08;
			}
			// 9
			if ((x >= 310) && (x <= 360))
			{
				waitForIt(310, 170, 360, 210);
				return 0x09;

			}
			// Backspace
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

/*
* No change returns 0xFF
* Accept returns 0xF1
* Cancel returns 0xF0
* Value contained in total
*/
uint8_t keypadControllerDec(uint8_t& index, uint16_t& total)
{
	uint8_t input = keypadButtonsDec();
	if (input >= 0x00 && input < 0x10 && index < 4)
	{
		keypadInput[3] = keypadInput[2];
		keypadInput[2] = keypadInput[1];
		keypadInput[1] = keypadInput[0];
		keypadInput[0] = input;
		total = keypadInput[0] * 1 + keypadInput[1] * 10 + keypadInput[2] * 100 + keypadInput[3] * 1000;
		drawRoundBtn(255, 220, 470, 260, String(total), menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
		++index;
		return 0xFF;
	}
	if (input == 0x10)
	{
		switch (index)
		{
		case 1:
			keypadInput[0] = 0;
			break;
		case 2:
			keypadInput[0] = keypadInput[1];
			keypadInput[1] = 0;
			break;
		case 3:
			keypadInput[0] = keypadInput[1];
			keypadInput[1] = keypadInput[2];
			keypadInput[2] = 0;
			break;
		case 4:
			keypadInput[0] = keypadInput[1];
			keypadInput[1] = keypadInput[2];
			keypadInput[2] = keypadInput[3];
			keypadInput[3] = 0;
			break;
		case 5:
			keypadInput[0] = keypadInput[1];
			keypadInput[1] = keypadInput[2];
			keypadInput[2] = keypadInput[3];
			keypadInput[3] = keypadInput[4];
			keypadInput[4] = 0;
			break;
		}
		total = keypadInput[0] * 1 + keypadInput[1] * 10 + keypadInput[2] * 100 + keypadInput[3] * 1000;
		drawRoundBtn(255, 220, 470, 260, String(total), menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
		(index > 0) ? --index : 0;
		return 0xFF;
	}
	if (input == 0x11)
	{
		return 0xF1;
	}
	else if (input == 0x12)
	{
		return 0xF0;
	}
	return input;
}
 
/*============== Keyboard ==============*/
// User input keypad
void drawkeyboard()
{
	drawSquareBtn(131, 55, 479, 319, "", themeBackground, themeBackground, themeBackground, CENTER);
	//SerialUSB.println("");
	uint16_t posY = 56;
	uint8_t numPad = 0x00;

	const char keyboardInput[37] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9',
									 'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j',
									 'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't',
									 'u', 'v', 'w', 'x', 'y', 'z', '_'};
	uint8_t count = 0;

	for (uint8_t i = 0; i < 4; i++)
	{
		int posX = 135;
		for (uint8_t j = 0; j < 10; j++)
		{
			if (count < 37)
			{
				drawRoundBtn(posX, posY, posX + 32, posY + 40, String(keyboardInput[count]), menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
				/* Print out button coords
				SerialUSB.print(posX);
				SerialUSB.print(" , ");
				SerialUSB.print(posY);
				SerialUSB.print(" , ");
				SerialUSB.print((posX + 32));
				SerialUSB.print(" , ");
				SerialUSB.println((posY + 40));
				*/
				posX += 34;
				count++;
			}
		}
		posY += 43;
	}
	drawRoundBtn(373, 185, 475, 225, F("<--"), menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
	drawRoundBtn(135, 230, 240, 270, F("Input:"), menuBackground, menuBtnBorder, menuBtnText, CENTER);
	drawRoundBtn(245, 230, 475, 270, F(""), menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
	drawRoundBtn(135, 275, 305, 315, F("Accept"), menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
	drawRoundBtn(310, 275, 475, 315, F("Cancel"), menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
}

// User input keyboard
int keyboardButtons()
{
	// Touch screen controls
	if (Touch_getXY())
	{
		if ((y >= 56) && (y <= 96))
		{
			if ((x >= 135) && (x <= 167))
			{
				waitForIt(135, 56, 167, 96);
				// 0
				DEBUG_KEYBOARD("0");
				return 0x30;
			}
			if ((x >= 169) && (x <= 201))
			{
				waitForIt(169, 56, 201, 96);
				// 1
				DEBUG_KEYBOARD("1");
				return 0x31;
			}
			if ((x >= 203) && (x <= 235))
			{
				waitForIt(203, 56, 235, 96);
				// 2
				DEBUG_KEYBOARD("2");
				return 0x32;
			}
			if ((x >= 237) && (x <= 269))
			{
				waitForIt(237, 56, 269, 96);
				// 3
				DEBUG_KEYBOARD("3");
				return 0x33;
			}
			if ((x >= 271) && (x <= 303))
			{
				waitForIt(271, 56, 303, 96);
				// 4
				DEBUG_KEYBOARD("4");
				return 0x34;
			}
			if ((x >= 305) && (x <= 337))
			{
				waitForIt(305, 56, 337, 96);
				// 5
				DEBUG_KEYBOARD("5");
				return 0x35;
			}
			if ((x >= 339) && (x <= 371))
			{
				waitForIt(339, 56, 371, 96);
				// 6
				DEBUG_KEYBOARD("6");
				return 0x36;
			}
			if ((x >= 373) && (x <= 405))
			{
				waitForIt(373, 56, 405, 96);
				// 7
				DEBUG_KEYBOARD("7");
				return 0x37;
			}
			if ((x >= 407) && (x <= 439))
			{
				waitForIt(407, 56, 439, 96);
				// 8
				DEBUG_KEYBOARD("8");
				return 0x38;
			}
			if ((x >= 441) && (x <= 473))
			{
				waitForIt(441, 56, 473, 96);
				// 9
				DEBUG_KEYBOARD("9");
				return 0x39;
			}
		}
		if ((y >= 99) && (y <= 139))
		{
			if ((x >= 135) && (x <= 167))
			{
				waitForIt(135, 99, 167, 139);
				// a
				DEBUG_KEYBOARD("a");
				return 0x61;
			}
			if ((x >= 169) && (x <= 201))
			{
				waitForIt(169, 99, 201, 139);
				// b
				DEBUG_KEYBOARD("b");
				return 0x62;
			}
			if ((x >= 203) && (x <= 235))
			{
				waitForIt(203, 99, 235, 139);
				// c
				DEBUG_KEYBOARD("c");
				return 0x63;
			}
			if ((x >= 237) && (x <= 269))
			{
				waitForIt(237, 99, 269, 139);
				// d
				DEBUG_KEYBOARD("d");
				return 0x64;
			}
			if ((x >= 271) && (x <= 303))
			{
				waitForIt(271, 99, 303, 139);
				// e
				DEBUG_KEYBOARD("e");
				return 0x65;
			}
			if ((x >= 305) && (x <= 337))
			{
				waitForIt(305, 99, 337, 139);
				// f
				DEBUG_KEYBOARD("f");
				return 0x66;
			}
			if ((x >= 339) && (x <= 371))
			{
				waitForIt(339, 99, 371, 139);
				// g
				DEBUG_KEYBOARD("g");
				return 0x67;
			}
			if ((x >= 373) && (x <= 405))
			{
				waitForIt(373, 99, 405, 139);
				// h
				DEBUG_KEYBOARD("h");
				return 0x68;
			}
			if ((x >= 407) && (x <= 439))
			{
				waitForIt(407, 99, 439, 139);
				// i
				DEBUG_KEYBOARD("i");
				return 0x69;
			}
			if ((x >= 441) && (x <= 473))
			{
				waitForIt(441, 99, 473, 139);
				// j
				DEBUG_KEYBOARD("j");
				return 0x6A;
			}
		}
		if ((y >= 142) && (y <= 182))
		{
			if ((x >= 135) && (x <= 167))
			{
				waitForIt(135, 142, 167, 182);
				// k
				DEBUG_KEYBOARD("k");
				return 0x6B;
			}
			if ((x >= 169) && (x <= 201))
			{
				waitForIt(169, 142, 201, 182);
				// l
				DEBUG_KEYBOARD("l");
				return 0x6C;
			}
			if ((x >= 203) && (x <= 235))
			{
				waitForIt(203, 142, 235, 182);
				// m
				DEBUG_KEYBOARD("m");
				return 0x6D;
			}
			if ((x >= 237) && (x <= 269))
			{
				waitForIt(237, 142, 269, 182);
				// n
				DEBUG_KEYBOARD("n");
				return 0x6E;
			}
			if ((x >= 271) && (x <= 303))
			{
				waitForIt(271, 142, 303, 182);
				// o
				DEBUG_KEYBOARD("o");
				return 0x6F;
			}
			if ((x >= 305) && (x <= 337))
			{
				waitForIt(305, 142, 337, 182);
				// p
				DEBUG_KEYBOARD("p");
				return 0x70;
			}
			if ((x >= 339) && (x <= 371))
			{
				waitForIt(339, 142, 371, 182);
				// q
				DEBUG_KEYBOARD("q");
				return 0x71;
			}
			if ((x >= 373) && (x <= 405))
			{
				waitForIt(373, 142, 405, 182);
				// r
				DEBUG_KEYBOARD("r");
				return 0x72;
			}
			if ((x >= 407) && (x <= 439))
			{
				waitForIt(407, 142, 439, 182);
				// s
				DEBUG_KEYBOARD("s");
				return 0x73;
			}
			if ((x >= 441) && (x <= 473))
			{
				waitForIt(441, 142, 473, 182);
				// t
				DEBUG_KEYBOARD("t");
				return 0x74;
			}
		}
		if ((y >= 185) && (y <= 225))
		{
			if ((x >= 135) && (x <= 167))
			{
				waitForIt(135, 185, 167, 225);
				// u
				DEBUG_KEYBOARD("u");
				return 0x75;
			}
			if ((x >= 169) && (x <= 201))
			{
				waitForIt(169, 185, 201, 225);
				// v
				DEBUG_KEYBOARD("v");
				return 0x76;
			}
			
			if ((x >= 203) && (x <= 235))
			{
				waitForIt(203, 185, 235, 225);
				// w
				DEBUG_KEYBOARD("w");
				return 0x77;
			}
			if ((x >= 237) && (x <= 269))
			{
				waitForIt(237, 185, 269, 225);
				// x
				DEBUG_KEYBOARD("x");
				return 0x78;
			}
			if ((x >= 271) && (x <= 303))
			{
				waitForIt(271, 185, 303, 225);
				// y
				DEBUG_KEYBOARD("y");
				return 0x79;
			}
			if ((x >= 305) && (x <= 337))
			{
				waitForIt(305, 185, 337, 225);
				// z
				DEBUG_KEYBOARD("z");
				return 0x7A;
			}
			if ((x >= 339) && (x <= 371))
			{
				waitForIt(339, 185, 371, 225);
				// z
				DEBUG_KEYBOARD("_");
				return 0x5F;
			}
			if ((x >= 373) && (x <= 474))
			{
				waitForIt(373, 185, 475, 225);
				// Backspace
				DEBUG_KEYBOARD("Backspace");
				return 0xF2;
			}
		}
		if ((y >= 275) && (y <= 315))
		{
			if ((x >= 135) && (x <= 305))
			{
				waitForIt(135, 275, 305, 315);
				// Accept
				DEBUG_KEYBOARD("Accept");
				return 0xF1;

			}
			if ((x >= 310) && (x <= 475))
			{
				waitForIt(310, 275, 475, 315);
				// Cancel
				DEBUG_KEYBOARD("Cancel");
				return 0xF0;
			}
		}
	}
	return 0xFF;
}

/*
* No change returns 0xFF
* Accept returns 0xF1
* Cancel returns 0xF0
* Value contained in global keyboardInput
*/
uint8_t keyboardController(uint8_t &index)
{
	uint8_t input = keyboardButtons();
	if (input > 0x29 && input < 0x7B && index < 8) // 8 is max size of a filename
	{
		keyboardInput[index] = input;
		/*
		SerialUSB.println("");
		SerialUSB.print("Index: ");
		SerialUSB.println(index);
		for (int i = 0; i < 8; i++)
		{
			SerialUSB.print(keyboardInput[i]);
			SerialUSB.print(" ");
		}
		SerialUSB.println("");
		*/
		drawRoundBtn(245, 230, 475, 270, String(keyboardInput), menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
		++index;
		return 0xFF;
	}
	if (input == 0xF2 && index > 0)
	{
		keyboardInput[index - 1] = 0x20;
		drawRoundBtn(245, 230, 475, 270, String(keyboardInput), menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
		--index;
		return 0xFF;
	}
	return input;
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

// Error Message buttons
uint8_t errorMSGButton(uint8_t returnPage)
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
				return 3;
			}
		}
		if ((y >= 180) && (y <= 215))
		{
			if ((x >= 155) && (x <= 275))
			{
				// Confirm
				waitForItRect(155, 180, 275, 215);
				return 1;
			}
			if ((x >= 285) && (x <= 405))
			{
				// Cancel
				waitForItRect(285, 180, 405, 215);
				return 2;
			}
		}
	}
	return 0;
}

/*=========================================================
	Background Processes
===========================================================*/
// Button functions for main menu
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

// Displays time on menu
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

// Able to call background process from blocked loop
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

// Calls pageControl with a value of 1 to set view page as the home page
void loop()
{
	// GUI
	pageControl();

	// Background Processes
	backgroundProcess();
}
