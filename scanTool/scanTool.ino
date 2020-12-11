/*
 Name:    scanTool.ino
 Created: 11/15/2020 8:27:18 AM
 Author:  Brandon Van Pelt
*/

#include <SD.h>
#include <UTouchCD.h>
#include <memorysaver.h>
#include <SPI.h>
#include <UTFT.h>
#include <UTouch.h>
#include "CANBus.h"
#include "definitions.h"
#include "icons.h"
#include "SDCard.h"

// Initialize display
//(byte model, int RS, int WR, int CS, int RST, int SER)
UTFT myGLCD(ILI9488_16, 7, 38, 9, 10);
//RTP: byte tclk, byte tcs, byte din, byte dout, byte irq
UTouch  myTouch(2, 6, 3, 4, 5);

// For touch controls
int x, y;

uint8_t controlPage = 1;

// External import for fonts
// extern uint8_t SmallFont[];
extern uint8_t BigFont[];

// Object to control CAN Bus hardware
CANBus can1;

// Object to control SD Card Hardware
SDCard sdCard;

// BITMAP global for bmpDraw()
int dispx, dispy;

// Used for PID scroll function
char currentDir[20];
uint8_t arrayIn[100];
bool hasPID = false;

/*=========================================================
    Framework Functions
===========================================================*/
//
void bmpDraw(char *filename, int x, int y) {
  File     bmpFile;
  int      bmpWidth, bmpHeight;   // W+H in pixels
  uint8_t  bmpDepth;              // Bit depth (currently must be 24)
  uint32_t bmpImageoffset;        // Start of image data in file
  uint32_t rowSize;               // Not always = bmpWidth; may have padding
  uint8_t  sdbuffer[3*BUFFPIXEL]; // pixel in buffer (R+G+B per pixel)
  uint16_t lcdbuffer[BUFFPIXEL];  // pixel out buffer (16-bit per pixel)
  uint8_t  buffidx = sizeof(sdbuffer); // Current position in sdbuffer
  boolean  goodBmp = false;       // Set to true on valid header parse
  boolean  flip    = true;        // BMP is stored bottom-to-top
  int      w, h, row, col;
  uint8_t  r, g, b;
  uint32_t pos = 0, startTime = millis();
  uint8_t  lcdidx = 0;
  boolean  first = true;

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
  if(read16(bmpFile) == 0x4D42) { // BMP signature

    Serial.println(read32(bmpFile));
    (void)read32(bmpFile); // Read & ignore creator bytes
    bmpImageoffset = read32(bmpFile); // Start of image data
    Serial.print(F("Image Offset: ")); 
    Serial.println(bmpImageoffset, DEC);

    // Read DIB header
    Serial.print(F("Header size: ")); 
    Serial.println(read32(bmpFile));
    bmpWidth  = read32(bmpFile);
    bmpHeight = read32(bmpFile);

    if(read16(bmpFile) == 1) { // # planes -- must be '1'
      bmpDepth = read16(bmpFile); // bits per pixel
      Serial.print(F("Bit Depth: ")); 
      Serial.println(bmpDepth);
      if((bmpDepth == 24) && (read32(bmpFile) == 0)) { // 0 = uncompressed
        goodBmp = true; // Supported BMP format -- proceed!
        Serial.print(F("Image size: "));
        Serial.print(bmpWidth);
        Serial.print('x');
        Serial.println(bmpHeight);

        // BMP rows are padded (if needed) to 4-byte boundary
        rowSize = (bmpWidth * 3 + 3) & ~3;

        // If bmpHeight is negative, image is in top-down order.
        // This is not canon but has been observed in the wild.
        if(bmpHeight < 0) {
          bmpHeight = -bmpHeight;
          flip      = false;
        }

        // Crop area to be loaded
        w = bmpWidth;
        h = bmpHeight;
        if((x+w-1) >= dispx)  w = dispx  - x;
        if((y+h-1) >= dispy) h = dispy - y;

        // Set TFT address window to clipped image bounds

        for (row=0; row<h; row++) { // For each scanline...
          // Seek to start of scan line.  It might seem labor-
          // intensive to be doing this on every line, but this
          // method covers a lot of gritty details like cropping
          // and scanline padding.  Also, the seek only takes
          // place if the file position actually needs to change
          // (avoids a lot of cluster math in SD library).
          if(flip) // Bitmap is stored bottom-to-top order (normal BMP)
            pos = bmpImageoffset + (bmpHeight - 1 - row) * rowSize;
          else     // Bitmap is stored top-to-bottom
          pos = bmpImageoffset + row * rowSize;
          if(bmpFile.position() != pos) { // Need seek?
            bmpFile.seek(pos);
            buffidx = sizeof(sdbuffer); // Force buffer reload
          }

          for (col=0; col<w; col++) { // For each column...
            // Time to read more pixel data?
            if (buffidx >= sizeof(sdbuffer)) { // Indeed
              // Push LCD buffer to the display first
              if(lcdidx > 0) {
                myGLCD.setColor(lcdbuffer[lcdidx]);
                myGLCD.drawPixel(col+155, row+1);
                lcdidx = 0;
                first  = false;
              }

              bmpFile.read(sdbuffer, sizeof(sdbuffer));
              buffidx = 0; // Set index to beginning
            }

            // Convert pixel from BMP to TFT format
            b = sdbuffer[buffidx++];
            g = sdbuffer[buffidx++];
            r = sdbuffer[buffidx++];
            myGLCD.setColor(r,g,b);
            myGLCD.drawPixel(col+155, row+1);
          } // end pixel

        } // end scanline

        // Write any remaining data to LCD
        if(lcdidx > 0) {
           myGLCD.setColor(lcdbuffer[lcdidx]);
          myGLCD.drawPixel(col+155, row+1);
        } 
      } // end goodBmp
    }
  }

  bmpFile.close();
  if(!goodBmp) Serial.println(F("BMP format not recognized."));

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

// Function complete load bar
void loadBar(int progress)
{
    if (progress >= DONE)
    {
        myGLCD.setColor(menuBtnColor);
        myGLCD.fillRect(200, 200, 400, 220);
        return;
    }
    drawSquareBtn(199, 199, 401, 221, "", themeBackground, menuBtnBorder, menuBtnText, LEFT);
    myGLCD.setColor(menuBtnColor);
    myGLCD.fillRect(200, 200, (200 + (progress * 25)), 220);
}

/*****************************************************
*  Draw Round/Square Button                          *
*                                                    *
*  Description:   Draws shapes with/without text     *
*                                                    *
*  Parameters: int: x start, y start, x stop, y stop *
*              String: Button text                   *
*              Hex value: Background Color           *
*              Hex value: Border of shape            *
*              Hex value: Color of text              *
*              int: Alignment of text #defined as    *
*                   LEFT, CENTER, RIGHT              *
*                                                    *
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

// Function to highlight buttons when clicked
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


/*=========================================================
    PID Scan Functions
===========================================================*/
// Draw the PID scan page
void drawPIDSCAN()
{
    drawSquareBtn(150, 60, 479, 319, "", themeBackground, themeBackground, themeBackground, CENTER);
    drawSquareBtn(141, 80, 479, 100, "Scan supported PIDs", themeBackground, themeBackground, menuBtnColor, CENTER);
    drawSquareBtn(141, 105, 479, 125, "to SD Card", themeBackground, themeBackground, menuBtnColor, CENTER);
    drawRoundBtn(200, 135, 400, 185, "Start Scan", menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
        return;
}

// Buttons for PID scan page
void PIDSCAN()
{
    while (true)
    {
        // Touch screen controls
        if (myTouch.dataAvailable())
        {
            myTouch.read();
            x = myTouch.getX();
            y = myTouch.getY();

            // Start Scan
            if ((x >= 200) && (x <= 400))
            {
                if ((y >= 135) && (y <= 185))
                {
                    waitForIt(200, 135, 400, 185);
                    startPIDSCAN();
                }
            }
        }
        return;
    }
}

// Function starts the PID scan
void startPIDSCAN()
{
    bool waiting = true;
    bool hasNext = true;
    uint8_t status = 1;
    uint8_t bank = 0;
    uint16_t txid = 0x7DF;
    uint16_t rxid = 0x7E8;
    byte test[8] = { 0x02, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
    byte test1[8] = { 0x02, 0x09, 0x02, 0x55, 0x55, 0x55, 0x55, 0x55 };
    byte test2[8] = { 0x30, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
    uint8_t range = 0x00;

    loadBar(status);
    can1.requestVIN(rxid, currentDir);

    //loadBar(status++);
    //sdCard.writeFileln(can1.getFullDir());

    loadBar(status++);
    while (hasNext)
    {
        can1.getPIDList(range, bank);
        loadBar(status++);
        delay(500);
        range = range + 0x20;
        bank++;
        hasNext = can1.getNextPID();
    }
    loadBar(DONE);
    hasPID = true;
    return;
}


/*=========================================================
    Home Functions
===========================================================*/
// Draw the view page
void drawHome()
{
    drawSquareBtn(145, 60, 479, 319, "", themeBackground, themeBackground, themeBackground, CENTER);
    drawSquareBtn(150, 300, 479, 319, "Scan Tool Ver. 1.1", themeBackground, themeBackground, menuBtnColor, CENTER);
}


/*=========================================================
    PID Stream Functions
===========================================================*/
// Function to print and scroll an array list
void drawProgramScroll(int scroll)
{
    // Temp to hold PIDS value before strcat
    char temp[2];

    // Starting y location for list
    int y = 60;

    // Draw the scroll window
    for (int i = 0; i < MAXSCROLL; i++)
    {
        char intOut[4] = "0x";
        itoa(arrayIn[scroll+1], temp, 16);
        strcat(intOut, temp);
        if (scroll < sizeof(arrayIn) && arrayIn[scroll+1] > 0)
        {
            drawSquareBtn(150, y, 410, y + 35, intOut, menuBackground, menuBtnBorder, menuBtnText, LEFT);
        }
        else
        {
            drawSquareBtn(150, y, 410, y + 35, "", menuBackground, menuBtnBorder, menuBtnText, LEFT);
        }
        y = y + 35;
        scroll++;
    }
}

// Draw the program page
void drawProgram(int scroll = 0)
{
    drawSquareBtn(141, 60, 479, 319, "", themeBackground, themeBackground, themeBackground, CENTER);
    int k = scroll;
    myGLCD.setColor(menuBtnColor);
    myGLCD.setBackColor(themeBackground);
    drawSquareBtn(420, 100, 470, 150, "/\\", menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
    drawSquareBtn(420, 150, 470, 200, "\\/", menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
    drawProgramScroll(scroll);
    drawRoundBtn(150, 275, 400, 315, "Stream PID", menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
}

// Button function for the program page
void program()
{
static int scroll = 0;
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
                Serial.println(1 + scroll);
            }
            if ((y >= 95) && (y <= 130))
            {
                waitForItRect(150, 95, 410, 130);
                Serial.println(2 + scroll);
            }
            if ((y >= 130) && (y <= 165))
            {
                waitForItRect(150, 130, 410, 165);
                Serial.println(3 + scroll);
            }
            if ((y >= 165) && (y <= 200))
            {
                waitForItRect(150, 165, 410, 200);
                Serial.println(4 + scroll);
            }
            if ((y >= 200) && (y <= 235))
            {
                waitForItRect(150, 200, 410, 235);
                Serial.println(5 + scroll);
            }
            if ((y >= 235) && (y <= 270))
            {
                waitForItRect(150, 235, 410, 270);
                Serial.println(6 + scroll);
            }
        }
        if ((x >= 420) && (x <= 470))
        {
            if ((y >= 100) && (y <= 150))
            {
                waitForItRect(420, 100, 470, 150);
                if (scroll > 0)
                {
                    scroll = scroll - 6;
                    drawProgramScroll(scroll);
                }
            }
        }
        if ((x >= 420) && (x <= 470))
        {
            if ((y >= 150) && (y <= 200))
            {
                waitForItRect(420, 150, 470, 200);
                if (scroll < 100)
                {
                    scroll = scroll + 6;
                    drawProgramScroll(scroll);
                }
            }
        }
    }
    return;
}


/*=========================================================
    Monitor CAN traffic
===========================================================*/
// Draw the config page
void drawTraffic()
{
    drawSquareBtn(145, 60, 479, 319, "", themeBackground, themeBackground, themeBackground, CENTER);
    drawSquareBtn(145, 140, 479, 160, "View CAN on serial", themeBackground, themeBackground, menuBtnColor, CENTER);
    return;
}


/*=========================================================
    Unused function
===========================================================*/
// Draw the send message page
void drawSendMSG()
{
    drawSquareBtn(145, 60, 479, 319, "", themeBackground, themeBackground, themeBackground, CENTER);
    //drawSquareBtn(145, 140, 479, 160, "Spare function", themeBackground, themeBackground, menuBtnColor, CENTER);

    return;
}


/*=========================================================
    Framework Functions
===========================================================*/
//Only called once at startup to draw the menu
void drawMenu()
{
    // Draw Layout
    drawSquareBtn(1, 1, 479, 319, "", themeBackground, themeBackground, themeBackground, CENTER);
    drawSquareBtn(1, 1, 140, 319, "", menuBackground, menuBackground, menuBackground, CENTER);

    // Draw Menu Buttons
    drawRoundBtn(10, 10, 130, 65, "HOME", menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
    drawRoundBtn(10, 70, 130, 125, "PIDSCAN", menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
    drawRoundBtn(10, 130, 130, 185, "PIDSTRM", menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
    drawRoundBtn(10, 190, 130, 245, "SENDMSG", menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
    drawRoundBtn(10, 250, 130, 305, "TRAFFIC", menuBtnColor, menuBtnBorder, menuBtnText, CENTER);
}

//Manages the different App pages
int pageControl(uint8_t page, bool value = false)
{
    
    bool waiting = true;

    static bool hasDrawn;
    static bool isExecute;
    hasDrawn = value;
    while (true)
    {
        menu();
        if (isExecute)
        {

        }
        switch (page)
        {
        case 1:
            if (!hasDrawn)
            {
                drawHome();
                hasDrawn = true;
                controlPage = page;
            }
            // Call buttons if any
            break;
        case 2:
            if (!hasDrawn)
            {
                drawPIDSCAN();
                hasDrawn = true;
                controlPage = page;
            }
            // Call buttons if any
            PIDSCAN();
            break;
        case 3:
            if (!hasDrawn)
            {
                if (hasPID == true)
                {
                    sdCard.readFile(can1.getFullDir(), arrayIn);
                    drawProgram();
                    hasDrawn = true;
                    controlPage = page;
                }
                else
                {
                    errorMSG("error", "Please Scan", "PIDS first");
                    hasDrawn = true;
                }
                
            }
            // Call buttons if any
            if (hasPID == true)
            {
                program();
            }
            else
            {
                errorMSGBtn(page);
            }
            break;
        case 4:
            if (!hasDrawn)
            {
                drawSendMSG();
                hasDrawn = true;
                controlPage = page;
            }
            // Call buttons if any

            break;
        case 5:
            if (!hasDrawn)
            {
                drawTraffic();
                hasDrawn = true;
                controlPage = page;
            }
            // Call buttons if any
            can1.CANTraffic();
            break;
        }
    }
}

// Error Message function
void errorMSG(String title, String eMessage1, String eMessage2)
{
    drawSquareBtn(170, 140, 450, 240, "", menuBackground, menuBtnColor, menuBtnColor, CENTER);
    drawSquareBtn(170, 140, 450, 170, title, themeBackground, menuBtnColor, menuBtnBorder, LEFT);
    drawSquareBtn(171, 171, 449, 204, eMessage1, menuBackground, menuBackground, menuBtnText, CENTER);
    drawSquareBtn(171, 205, 449, 239, eMessage2, menuBackground, menuBackground, menuBtnText, CENTER);
    drawRoundBtn(400, 140, 450, 170, "X", menuBtnColor, menuBtnColor, menuBtnText, CENTER);
}

uint8_t errorMSGBtn(uint8_t page)
{
    // Touch screen controls
    if (myTouch.dataAvailable())
    {
        myTouch.read();
        x = myTouch.getX();
        y = myTouch.getY();

        if ((x >= 400) && (x <= 450))
        {
            if ((y >= 140) && (y <= 170))
            {
                waitForItRect(400, 140, 450, 170);
                pageControl(controlPage);
            }
        }
    }
}

// Button functions for main menu
void menu()
{
    while (true)
    {

        // Physical Button control
        if (digitalRead(A4) == HIGH)
        {
            //drawLoadProgram();
            drawMenu();
            delay(BUTTON_DELAY);
        }
        if (digitalRead(A3) == HIGH)
        {
            //drawManualControl();
            delay(BUTTON_DELAY);
            drawMenu();
        }
        if (digitalRead(A2) == HIGH)
        {
            //drawSettings();
            delay(BUTTON_DELAY);
            drawMenu();
        }
        if (digitalRead(A1) == HIGH)
        {
            //runProgram(*programPtr);
            delay(BUTTON_DELAY);
            drawMenu();
        }
        if (digitalRead(A0) == HIGH)
        {
            delay(BUTTON_DELAY);
        }

        // Touch screen controls
        if (myTouch.dataAvailable())
        {
            myTouch.read();
            x = myTouch.getX();
            y = myTouch.getY();

            // Menu
            if ((x >= 10) && (x <= 130))  // Button: 1
            {
                if ((y >= 10) && (y <= 65))  // Upper row
                {
                    waitForIt(10, 10, 130, 65);
                    pageControl(1);
                }
                if ((y >= 70) && (y <= 125))  // Upper row
                {

                    // X_Start, Y_Start, X_Stop, Y_Stop
                    waitForIt(10, 70, 130, 125);
                    pageControl(2);

                }
                if ((y >= 130) && (y <= 185))  // Upper row
                {
                    // X_Start, Y_Start, X_Stop, Y_Stop
                    waitForIt(10, 130, 130, 185);
                    pageControl(3);
                }
                // Settings touch button
                if ((y >= 190) && (y <= 245))
                {

                    // X_Start, Y_Start, X_Stop, Y_Stop
                    waitForIt(10, 190, 130, 245);
                    pageControl(4);
                }
                if ((y >= 250) && (y <= 305))
                {

                    // X_Start, Y_Start, X_Stop, Y_Stop
                    waitForIt(10, 250, 130, 305);
                    pageControl(5);
                }

            }
        }
        return;
    }
}

// the setup function runs once when you press reset or power the board
void setup() {
    Serial.begin(115200);

    can1.startCAN();
    bool hasFailed = sdCard.startSD();
    if (!hasFailed)
    {
        Serial.println("SD failed");
    }
    else if (hasFailed)
    {
        Serial.println("SD Running");
    }

    // LCD  and touch screen settings
    myGLCD.InitLCD();
    myGLCD.clrScr();

    myTouch.InitTouch();
    myTouch.setPrecision(PREC_MEDIUM);

    myGLCD.setFont(BigFont);
    myGLCD.setBackColor(0, 0, 255);

    dispx = myGLCD.getDisplayXSize();
    dispy = myGLCD.getDisplayYSize();

    // Draw the main menu
    drawMenu();

    // Draw the Hypertech logo
    bmpDraw("HYPER.bmp", 0, 0);
}

// Calls pageControl with a value of 1 to set view page as the home page
void loop() {
    
    controlPage = pageControl(controlPage);

}