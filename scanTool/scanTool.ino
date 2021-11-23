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
Switch to FATSD
-RTC Time stamps
File naming
Organize / move output variables into a settings structure
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
char keyboardInput[10] = {' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '};
uint8_t keypadInput[4] = {0, 0, 0, 0};

// Used for converting keypad input to appropriate hex place
const uint32_t hexTable[8] = { 1, 16, 256, 4096, 65536, 1048576, 16777216, 268435456 };

// List of baud rates for Baud page
const uint32_t baudRates[6] = { 1000000, 800000, 500000, 250000, 125000, 100000 };

// Filter range / Filter Mask
uint32_t CAN0Filter = 0x000;
uint32_t CAN0Mask = 0x7FF;
uint32_t CAN1Filter = 0x000;
uint32_t CAN1Mask = 0x7FF;

// Holds CAN Bus capture replay filenames
char fileList[10][13];

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

// Manages the different App pages
void pageControl()
{
	// Check menu buttons for input
	menuButtons();
	switch (page)
	{
	case 0: /*========== CANBUS ==========*/
		// Draw Page
		if (!hasDrawn)
		{
			if (drawCANBus())
			{
				graphicLoaderState++;
			}
			else
			{
				hasDrawn = true;
			}
		}

		// Call buttons if any
		CANBusButtons();

		// Release any variable locks if page changed
		if (nextPage != page)
		{
			hasDrawn = false;
			page = nextPage;
		}
		break;

	case 1: // Capture
		switch (state)
		{
		case 0:
			if (!hasDrawn && graphicLoaderState < 1)
			{
				bool error = false;
				(lockVar16(LOCK0)) ? g_var16[POS0] = 0 : error = true;
				if (error)
				{
					DEBUG_ERROR("Error: Variable locked");
					nextPage = 0;
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

		// Call buttons if any
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
		// Draw Page
		if (!hasDrawn)
		{
			if (drawCANLog())
			{
				graphicLoaderState++;
				break;
			}

			bool error = false;
			(lockVar8(LOCK0)) ? g_var8[POS0] = 0 : error = true;
			(lockVar8(LOCK1)) ? g_var8[POS1] = 0 : error = true;
			(lockVar16(LOCK0)) ? g_var16[POS0] = 0 : error = true;
			state = 0;
			if (error)
			{
				DEBUG_ERROR("Error: Variable locked");
				nextPage = 0;
			}
			hasDrawn = true;
			drawCANLogScroll();
		}

		// Call buttons if any
		if (state == 0)
		{
			CANLogButtons();
		}
		else if (state == 1)
		{
			char fileLocation[20] = "CANLOG/";
			strcat(fileLocation, fileList[g_var16[POS0] - 1]);
			uint8_t input = errorMSGButton(2);
			switch (input)
			{
			case 1:
				state = 0;
				sdCard.deleteFile(fileLocation);
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

	case 3: // Send CAN Frame
		// Draw Page
		if (!hasDrawn)
		{
			bool error = false;
			// Lock global variables
			// lockVar8(LOCK0) called from button
			(lockVar8(LOCK0)) ? g_var8[POS0] = 0 : error = true; // Channel
			(lockVar8(LOCK1)) ? g_var8[POS1] = 0 : error = true; // keypad return 
			(lockVar8(LOCK2)) ? g_var8[POS2] = 0 : error = true; // keypad index
			(lockVar16(LOCK0)) ? g_var16[POS0] = 0 : error = true;
			(lockVar16(LOCK1)) ? g_var16[POS1] = 0 : error = true;
			(lockVar16(LOCK2)) ? g_var16[POS2] = 0 : error = true; // keypad input total
			state = 0;
			isFinished = false;
			if (error)
			{
				DEBUG_ERROR("Error: Variable locked");
				nextPage = 0;
			}
			hasDrawn = true;
		}

		// Call buttons if any
		sendCANFrame(g_var8[POS0]);

		// Release variable Locks if page changed
		if (nextPage != page)
		{
			unlockVar8(LOCK0);
			unlockVar8(LOCK1);
			unlockVar8(LOCK2);
			unlockVar16(LOCK0);
			unlockVar16(LOCK1);
			unlockVar16(LOCK2);
			hasDrawn = false;
			page = nextPage;
		}
		break;

	case 4: // Filter Mask
		// Draw Page
		if (!hasDrawn)
		{
			bool error = false;
			// Lock global variables
			(lockVar16(LOCK0)) ? g_var16[POS0] = 5 : error = true;
			(lockVar32(LOCK0)) ? g_var32[POS0] = 0 : error = true;
			(lockVar32(LOCK1)) ? g_var32[POS1] = 0 : error = true;
			if (error)
			{
				DEBUG_ERROR("Error: Variable locked");
				nextPage = 0;
			}
			hasDrawn = true;
			state = 0;
			isFinished = false;
		}

		// Call buttons if any
		filterMask();

		// Release any variable locks if page changed
		if (nextPage != page)
		{
			unlockVar16(LOCK0);
			unlockVar32(LOCK0);
			unlockVar32(LOCK1);
			hasDrawn = false;
			page = nextPage;
		}
		break;

	case 5: // Set Baud
		// Draw Page
		if (!hasDrawn)
		{
			if (drawBaud())
			{
				graphicLoaderState++;
				break;
			}

			bool error = false;
			(lockVar32(LOCK0)) ? g_var32[POS0] = 0 : error = true;
			if (error)
			{
				DEBUG_ERROR("Error: Variable locked");
				nextPage = 0;
			}
			drawBaudScroll();
			drawCurrentBaud();
			hasDrawn = true;
		}

		// Call buttons if any
		baudButtons();

		// Release any variable locks if page changed
		if (nextPage != page)
		{
			unlockVar32(LOCK0);
			hasDrawn = false;
			page = nextPage;
		}
		break;

	case 6: // Timed RX
		// Draw Page
		if (!hasDrawn)
		{
			if (drawTimedTX())
			{
				graphicLoaderState++;
				break;
			}

			bool error = false;
			(lockVar8(LOCK0)) ? g_var8[POS0] = 0 : error = true; // Selected index
			(lockVar8(LOCK1)) ? g_var8[POS1] = 0 : error = true; // User input
			(lockVar8(LOCK2)) ? g_var8[POS2] = 0 : error = true; // Keypad index
			(lockVar8(LOCK3)) ? g_var8[POS3] = 0 : error = true; // Scroll index
			(lockVar8(LOCK4)) ? g_var8[POS4] = 0 : error = true; // Node position
			(lockVar16(LOCK0)) ? g_var16[POS0] = 0 : error = true; // Total value
			if (error)
			{
				DEBUG_ERROR("Error: Variable locked");
				nextPage = 0;
			}
			state = 0;
			hasDrawn = true;
		}
		// Call buttons if any
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
			hasDrawn = false;
			page = nextPage;
		}
		break;

	case 7: // LCD Capture
		// Draw Page
		if (!hasDrawn)
		{
			hasDrawn = true;
			drawReadInCANLCD();
			isSerialOut = false;
			Can0.empty_rx_buff();
			Can1.empty_rx_buff();
		}

		// Call buttons if any
		readInCANMsg(selectedChannelOut);

		// Release any variable locks if page changed
		if (nextPage != page)
		{
			hasDrawn = false;
			page = nextPage;
		}
		break;

	case 8: // SD Capture
		// Draw Page
		if (!hasDrawn)
		{
			isSerialOut = false;
			Can0.empty_rx_buff();
			Can1.empty_rx_buff();
			isSDOut = true;
			hasDrawn = true;
		}

		// Call buttons if any

		// Release any variable locks if page changed
		if (nextPage != page)
		{
			hasDrawn = false;
			page = nextPage;
		}
		break;

	case 9: /*========== VEHTOOL ==========*/
		// Draw Page
		if (!hasDrawn)
		{
			if (drawVehicleTools())
			{
				graphicLoaderState++;
			}
			else
			{
				hasDrawn = true;
			}
		}

		// Call buttons if any
		VehicleToolButtons();

		// Release any variable locks if page changed
		if (nextPage != page)
		{
			hasDrawn = false;
			page = nextPage;
		}
		break;

	case 10: // PIDSCAN
		// Draw Page
		if (!hasDrawn)
		{
			if (hasPID)
			{
				page = 9;
				break;
			}

			// Lock global variables
			bool error = false;
			(lockVar8(LOCK0)) ? g_var8[POS0] = 0 : error = true;
			(lockVar8(LOCK1)) ? g_var8[POS1] = 0 : error = true;
			(lockVar16(LOCK0)) ? g_var16[POS0] = 0 : error = true;
			(lockVar16(LOCK1)) ? g_var16[POS1] = 0 : error = true;
			(lockVar32(LOCK0)) ? g_var32[POS0] = 0 : error = true;
			if (error)
			{
				DEBUG_ERROR("Error: Variable locked");
				nextPage = 9;
			}

			// Initialize state machine variables to 0
			state = 0;

			//can1.setFilterMask0(0x7E0, 0x7F0);
			//can1.startCAN0(0x7E0, 0x7EF);
			drawPIDSCAN();
			loadBar(g_var8[POS1]++);
			isSerialOut = false;
			hasDrawn = true;
		}

		// Call buttons if any
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
		// Draw Page
		if (!hasDrawn)
		{
			if (hasPID == true)
			{
				state = 0;
				// Lock global variables
				bool error = false;
				(lockVar8(LOCK0)) ? g_var8[POS0] = 0 : error = true;
				(lockVar16(LOCK0)) ? g_var16[POS0] = 0 : error = true;
				(lockVar16(LOCK1)) ? g_var16[POS1] = 0 : error = true;
				(lockVar32(LOCK0)) ? g_var32[POS0] = 0 : error = true;
				if (error)
				{
					DEBUG_ERROR("Error: Variable locked");
					nextPage = 9;
				}

				isSerialOut = false;
				can1.setFilterMask0(0x7E0, 0x1F0);

				for (int i = 0; i < 80; i++)
				{
					arrayIn[i] = 0x00;
				}
				sdCard.readFile(can1.getFullDir(), arrayIn);

				drawPIDStream();
				hasDrawn = true;
			}
			else
			{
				nextPage = 9;
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
		if ((state == 1) && (g_var16[POS1] < PIDSAMPLES) && (millis() - g_var32[POS0] > 1000))
		{
			// TODO: Fix me
			//can1.PIDStream(CAN_PID_ID, arrayIn[g_var16[POS0]], true);
			g_var16[POS1]++;
			drawErrorMSG("Samples", String(g_var16[POS1]), "Saved to SD");
			g_var32[POS0] = millis();
		}
		if ((g_var16[POS1] == PIDSAMPLES) && (state == 1) && (millis() - g_var32[POS0] > 2000))
		{
			state = 0;
			g_var8[POS0] = 0;
			drawPIDStream();
			g_var32[POS0] = millis(); //
		}

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
		// Draw Page
		if (!hasDrawn)
		{
			drawPIDGauges();
			hasDrawn = true;
			//can1.startPID();
		}

		// Call buttons if any
		PIDGauges();

		// Release any variable locks if page changed
		if (nextPage != page)
		{
			hasDrawn = false;
			page = nextPage;
		}
		break;

	case 13: // VIN
		// Draw Page
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

		// Call buttons if any

		// Release any variable locks if page changed
		if (nextPage != page)
		{
			can1.setFilterMask0(0x0, 0x0);
			hasDrawn = false;
			page = nextPage;
		}
		break;

	case 14: // 
		// Draw Page
		if (!hasDrawn)
		{

			hasDrawn = true;
		}

		// Call buttons if any

		// Release any variable locks if page changed
		if (nextPage != page)
		{
			hasDrawn = false;
			page = nextPage;
		}
		break;

	case 15: // DTC
		// Draw Page
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
				bool error = false;
				(lockVar16(LOCK1)) ? g_var16[POS1] = 0 : error = true;
				(lockVar32(LOCK0)) ? g_var32[POS0] = 0 : error = true;
				if (error)
				{
					DEBUG_ERROR("Error: Variable locked");
					nextPage = 9;
				}

				// Initialize state machine variables to 0
				hasDrawn = true;
				state = 0;
				isFinished = false;
			}
		}

		// Call buttons if any
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
		// Draw Page
		if (!hasDrawn)
		{
			hasDrawn = true;
		}

		// Call buttons if any

		// Release any variable locks if page changed
		if (nextPage != page)
		{
			hasDrawn = false;
			page = nextPage;
		}
		break;

	case 17:
		// Draw Page
		if (!hasDrawn)
		{
			hasDrawn = true;
		}
		// Call buttons if any

		// Release any variable locks if page changed
		if (nextPage != page)
		{
			hasDrawn = false;
			page = nextPage;
		}
		break;

	case 18: /*========== RZRTOOL ==========*/
		// Draw Page
		if (!hasDrawn)
		{
			if (drawRZRTOOL())
			{
				graphicLoaderState++;
			}
			else
			{
				hasDrawn = true;
			}
		}

		// Call buttons if any
		RZRToolButtons();

		// Release any variable locks if page changed
		if (nextPage != page)
		{
			hasDrawn = false;
			page = nextPage;
		}
		break;

	case 19: //
		// Draw Page
		if (!hasDrawn)
		{
			hasDrawn = true;
		}

		// Call buttons if any

		// Release any variable locks if page changed
		if (nextPage != page)
		{
			hasDrawn = false;
			page = nextPage;
		}
		break;

	case 20: //
		// Draw Page
		if (!hasDrawn)
		{
			hasDrawn = true;
		}

		// Call buttons if any

		// Release any variable locks if page changed
		if (nextPage != page)
		{
			hasDrawn = false;
			page = nextPage;
		}
		break;

	case 21: //
		// Draw Page
		if (!hasDrawn)
		{
			hasDrawn = true;
		}

		// Call buttons if any

		// Release any variable locks if page changed
		if (nextPage != page)
		{
			hasDrawn = false;
			page = nextPage;
		}
		break;

	case 22: //
		// Draw Page
		if (!hasDrawn)
		{
			hasDrawn = true;
		}

		// Call buttons if any

		// Release any variable locks if page changed
		if (nextPage != page)
		{
			hasDrawn = false;
			page = nextPage;
		}
		break;

	case 23: //
		// Draw Page
		if (!hasDrawn)
		{
			hasDrawn = true;
		}

		// Call buttons if any

		// Release any variable locks if page changed
		if (nextPage != page)
		{
			hasDrawn = false;
			page = nextPage;
		}
		break;

	case 24: //
		// Draw Page
		if (!hasDrawn)
		{
			hasDrawn = true;
		}

		// Call buttons if any

		// Release any variable locks if page changed
		if (nextPage != page)
		{
			hasDrawn = false;
			page = nextPage;
		}
		break;

	case 25: //
		// Draw Page
		if (!hasDrawn)
		{
			hasDrawn = true;
		}

		// Call buttons if any

		// Release any variable locks if page changed
		if (nextPage != page)
		{
			hasDrawn = false;
			page = nextPage;
		}
		break;

	case 26: //
		// Draw Page
		if (!hasDrawn)
		{
			hasDrawn = true;
		}

		// Call buttons if any

		// Release any variable locks if page changed
		if (nextPage != page)
		{
			hasDrawn = false;
			page = nextPage;
		}
		break;

	case 27: /*========== EXTRAFN ==========*/
		// Draw Page
		if (!hasDrawn)
		{
			if (drawExtraFN())
			{
				graphicLoaderState++;
			}
			else
			{
				hasDrawn = true;
			}
		}

		// Call buttons if any
		extraFNButtons();

		// Release any variable locks if page changed
		if (nextPage != page)
		{
			hasDrawn = false;
			page = nextPage;
		}
		break;

	case 28: // OBD CAN Simulator
		// Draw Page
		if (!hasDrawn)
		{
			drawOBDSimulator();
			hasDrawn = true;
		}

		// Call buttons if any
		entry();

		// Release any variable locks if page changed
		if (nextPage != page)
		{
			hasDrawn = false;
			page = nextPage;
		}
		break;

	case 29: // Message Spam
		// Draw Page
		if (!hasDrawn)
		{
			if (graphicLoaderState == 1)
			{
				// Lock global variables
				bool error = false;
				(lockVar8(LOCK1)) ? g_var8[POS1] = 0 : error = true; // Keypad index
				(lockVar16(LOCK0)) ? g_var16[POS0] = 0 : error = true; // ID
				(lockVar16(LOCK1)) ? g_var16[POS1] = 0x7FF : error = true; // Max
				(lockVar16(LOCK2)) ? g_var16[POS2] = 0x000 : error = true; // Min
				(lockVar16(LOCK3)) ? g_var16[POS3] = 30 : error = true; // Interval (ms)
				(lockVar16(LOCK4)) ? g_var16[POS4] = 30 : error = true; // Keypad total
				(lockVar32(LOCK0)) ? g_var32[POS0] = 0 : error = true; // Timer
				if (error)
				{
					DEBUG_ERROR("Error: Variable locked");
					nextPage = 27;
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

		// Call buttons if any
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
		// Draw Page
		if (!hasDrawn)
		{
			drawkeyboard();
			hasDrawn = true;
			g_var8[POS0] = 0;
			g_var8[POS1] = 0;
		}

		// Call buttons if any
		g_var8[POS0] = keyboardController(g_var8[POS1]);
		
		// Release any variable locks if page changed
		if (nextPage != page)
		{
			hasDrawn = false;
			page = nextPage;
		}
		break;

	case 31: //
		// Draw Page
		if (!hasDrawn)
		{
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

		// Call buttons if any

		// Release any variable locks if page changed
		if (nextPage != page)
		{
			hasDrawn = false;
			page = nextPage;
		}
		break;

	case 32: //
		// Draw Page
		if (!hasDrawn)
		{
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

		// Call buttons if any

		// Release any variable locks if page changed
		if (nextPage != page)
		{
			hasDrawn = false;
			page = nextPage;
		}
		break;

	case 33: //
		// Draw Page
		if (!hasDrawn)
		{
			isSerialOut = true;
			selectedChannelOut = 6;
			hasDrawn = true;
		}

		// Call buttons if any

		// Release any variable locks if page changed
		if (nextPage != page)
		{
			hasDrawn = false;
			page = nextPage;
		}
		break;

	case 34: // Ford Dongle
		// Draw Page
		if (!hasDrawn)
		{
			if (drawDongleSimFord())
			{
				graphicLoaderState++;
				break;
			}

			// Lock global variables
			bool error = false;
			(lockVar32(LOCK0)) ? g_var32[POS0] = 0 : error = true;
			if (error)
			{
				DEBUG_ERROR("Error: Variable locked");
				nextPage = 9;
			}
			hasDrawn = true;
		}
		
		// Call buttons if any
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
		// Draw Page
		if (!hasDrawn)
		{
			if (drawDongleSim())
			{
				graphicLoaderState++;
				break;
			}

			// Lock global variables
			bool error = false;
			(lockVar32(LOCK0)) ? g_var32[POS0] = 0 : error = true;
			if (error)
			{
				DEBUG_ERROR("Error: Variable locked");
				nextPage = 9;
			}

			hasDrawn = true;
		}

		// Call buttons if any
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
		// Draw Page
		if (!hasDrawn)
		{
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

		// Release any variable locks if page changed
		if (nextPage != page)
		{
			hasDrawn = false;
			page = nextPage;
		}
		break;

	case 37: // Memory
		// Draw Page
		if (!hasDrawn)
		{
			memoryUse();
			hasDrawn = true;
		}

		// Call buttons if any

		// Release any variable locks if page changed
		if (nextPage != page)
		{
			hasDrawn = false;
			page = nextPage;
		}
		break;

	case 38: //
		// Draw Page
		if (!hasDrawn)
		{
			hasDrawn = true;
		}

		// Call buttons if any

		// Release any variable locks if page changed
		if (nextPage != page)
		{
			hasDrawn = false;
			page = nextPage;
		}
		break;

	case 39: //
		// Draw Page
		if (!hasDrawn)
		{
			hasDrawn = true;
		}

		// Call buttons if any

		// Release any variable locks if page changed
		if (nextPage != page)
		{
			hasDrawn = false;
			page = nextPage;
		}
		break;

	case 40: //
		// Draw Page
		if (!hasDrawn)
		{
			// Draw Page
			hasDrawn = true;
		}

		// Call buttons if any

		// Release any variable locks if page changed
		if (nextPage != page)
		{
			hasDrawn = false;
			page = nextPage;
		}
		break;

	case 41: //
		// Draw Page
		if (!hasDrawn)
		{
			hasDrawn = true;
		}

		// Call buttons if any

		// Release any variable locks if page changed
		if (nextPage != page)
		{
			hasDrawn = false;
			page = nextPage;
		}
		break;

	case 42: //
		// Draw Page
		if (!hasDrawn)
		{
			hasDrawn = true;
		}

		// Call buttons if any

		// Release any variable locks if page changed
		if (nextPage != page)
		{
			hasDrawn = false;
			page = nextPage;
		}
		break;

	case 43: //
		// Draw Page
		if (!hasDrawn)
		{
			// Draw Page
			hasDrawn = true;
		}

		// Call buttons if any

		// Release any variable locks if page changed
		if (nextPage != page)
		{
			hasDrawn = false;
			page = nextPage;
		}
		break;

	case 44: //
		// Draw Page
		if (!hasDrawn)
		{
			hasDrawn = true;
		}

		// Call buttons if any

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

	// Draw the Hypertech logo
	bmpDraw("System/HYPER.bmp", 0, 0);
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

	const char keyboardInput[36] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9',
									 'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j',
									 'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't',
									 'u', 'v', 'w', 'x', 'y', 'z' };
	uint8_t count = 0;

	for (uint8_t i = 0; i < 4; i++)
	{
		int posX = 135;
		for (uint8_t j = 0; j < 10; j++)
		{
			if (count < 36)
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
	drawRoundBtn(340, 185, 474, 225, F("<--"), menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
	drawRoundBtn(135, 230, 240, 270, F("Input:"), menuBackground, menuBtnBorder, menuBtnText, CENTER);
	drawRoundBtn(245, 230, 475, 270, F("filename"), menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
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
			if ((x >= 340) && (x <= 474))
			{
				waitForIt(340, 185, 474, 225);
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
	if (input > 0x29 && input < 0x7B)
	{
		keyboardInput[index] = input;
		for (int i = 0; i < 10; i++)
		{
			SerialUSB.println(keyboardInput[i]);
		}
		drawRoundBtn(245, 230, 475, 270, String(keyboardInput), menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
		++index;
		return 0xFF;
	}
	if (input == 0xF2)
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
// Displays time on menu
void updateTime()
{
	if (millis() - updateClock > 1000)
	{
		char time[40];
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
	updateTime();
	serialOut();
	SDCardOut();
	timedTXSend();
}


/*=========================================================
	Main loop
===========================================================*/
uint32_t testtimers = 0;
// Calls pageControl with a value of 1 to set view page as the home page
void loop()
{
	// SD Card testing
	/*
	if (millis() - testtimers > 1000)
	{
		SerialUSB.println(Can0.available());
		testtimers = millis();
	}
	*/
	// GUI
	pageControl();

	// Background Processes
	backgroundProcess();
}


/*
irqLock();
{
	result = removeFromRingBuffer(rxRing, msg) ? 1 : 0;
}
irqRelease();
*/