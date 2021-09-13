// Used PID functions
uint8_t arrayIn[80];

uint8_t scroll = 0;

// Performing a sucessful PID scan will change this to true
bool hasPID = false;

// Filter range / Filter Mask
uint32_t CAN0Filter = 0x000;
uint32_t CAN0Mask = 0xFFF;
uint32_t CAN1Filter = 0x000;
uint32_t CAN1Mask = 0xFFF;

// General use variables
// Any non-background process function can use
// Initialize to 0 before use
bool nextState = false;
bool isFinished = false;
bool isSerialOut = false;
uint8_t state = 0;
int16_t counter1 = 0;
uint16_t var1 = 0;
uint8_t var2 = 0;
uint16_t var3 = 0;
uint32_t var4 = 0;
uint32_t var5 = 0;
uint32_t timer1 = 0;

// Use to load pages in pieces to prevent blocking while loading entire page
uint8_t graphicLoaderState = 0;

//
uint32_t waitForItTimer = 0;
uint16_t x1_ = 0;
uint16_t y1_ = 0;
uint16_t x2_ = 0;
uint16_t y2_ = 0;
bool isWaitForIt = false;

// Used for converting keypad input to appropriate hex place
const uint32_t hexTable[8] = { 1, 16, 256, 4096, 65536, 1048576, 16777216, 268435456 };