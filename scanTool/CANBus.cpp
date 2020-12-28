// CANBus manages the CAN bus hardware

#include "CANBus.h"
#include <due_can.h>
#include "variant.h"
#include "PIDS.h"

// Checks a single bit of binary number
#define CHECK_BIT(var,pos) ((var) & (1<<(pos)))

char VINLOG[7] = { 'L', 'O', 'G', '.', 't', 'x', 't' };
char PID[7] = { 'P', 'I', 'D', '.', 't', 'x', 't' };

// Constructor
CANBus::CANBus()
{

}

// Initialize CAN1 and set the proper baud rates here
void CANBus::startCAN()
{
    
    Can0.begin(CAN_BPS_500K);
    Can0.watchForRange(0x7E0, 0x7EF);
    return;
}

// CAN Bus send message method
void CANBus::sendFrame(uint16_t id, byte* frame)
{
    // Create message object
    CAN_FRAME sending;

    // Outgoing message ID
    sending.id = id;

    // Do not use extended frames
    sending.extended = false;

    // Message length
    sending.length = 8;

    // Assign object to message array
    sending.data.byte[0] = frame[0];
    sending.data.byte[1] = frame[1];
    sending.data.byte[2] = frame[2];
    sending.data.byte[3] = frame[3];
    sending.data.byte[4] = frame[4];
    sending.data.byte[5] = frame[5];
    sending.data.byte[6] = frame[6];
    sending.data.byte[7] = frame[7];

    Can0.sendFrame(sending);
    return;
}

// Method used for CAN recording
void CANBus::getPIDList(uint8_t range, uint8_t bank)
{
    // Create object to save message
    CAN_FRAME incoming;

    // Boolean to control recieve message while loop
    bool isWait = true;

    // Tx and Rx Ids
    uint16_t txid = 0x7DF;
    uint16_t IDFilter = 0x7E8;

    // Message used to requrest range of PIDS
    byte requestVIN[8] = { 0x02, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
    requestVIN[2] = range;
    sendFrame(txid, requestVIN);
    while (isWait)
    {
        if (Can0.available() > 0) {
            Can0.read(incoming);
            if (incoming.id == IDFilter)
            {
                // End loop since message was recieved
                isWait = false;

                // Log PID message
                SDPrint.writeFile(fullDir, "RX ID: ");
                SDPrint.writeFile(fullDir, incoming.id, HEX);
                SDPrint.writeFile(fullDir, " MSG: ");
                for (int count = 0; count < incoming.length; count++)
                {
                    SDPrint.writeFile(fullDir, incoming.data.bytes[count], HEX);
                    SDPrint.writeFile(fullDir, " ");
                }
                SDPrint.writeFileln(fullDir);

                // Check to see if the vehicle supports more PIDS
                setNextPID(CHECK_BIT(incoming.data.byte[6], 0));

                // Write the values from the bank if the Corresponding bit matches
                int pos = 0;
                for (int i = 3; i < 7; i++)
                {
                    for (int j = 7; j >= 0; j--)
                    {
                        if (CHECK_BIT(incoming.data.bytes[i], j))
                        {
                            SDPrint.writeFile(PIDDir, "0x");
                            SDPrint.writeFile(PIDDir, PID_bank[bank][pos], HEX);
                            SDPrint.writeFileln(PIDDir);
                        }
                        pos++;
                    }
                }
            }
        }
    }
    return;
}


void CANBus::requestVIN(uint16_t IDFilter, char* currentDir)
{
    // Create object to save message
    CAN_FRAME incoming;

    // Dir temp
    char temp[20];

    // Frames to request VIN
    uint8_t ReadVIN1st[8] = { 0x02, 0x09, 0x02, 0x55, 0x55, 0x55, 0x55, 0x55 };
    uint8_t ReadVIN_2nd[8] = { 0x30, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

    // Send ID
    uint16_t txidVIN = 0x7E0;

    // Waits to recieve all messages
    bool isWait = true;

    // Send first frame requesting VIN
    sendFrame(txidVIN, ReadVIN1st);
    while (isWait)
    {
        if (Can0.available() > 0) {
            Can0.read(incoming);
            if ((incoming.id == IDFilter) && (incoming.data.bytes[0] == 0x10))
            {
                VIN[0] = incoming.data.bytes[5];
                VIN[1] = incoming.data.bytes[6];
                VIN[2] = incoming.data.bytes[7];

                // Send second frame to continue VIN request
                sendFrame(txidVIN, ReadVIN_2nd);
            }
            if ((incoming.id == IDFilter) && (incoming.data.bytes[0] == 0x21))
            {
                VIN[3] = incoming.data.bytes[1];
                VIN[4] = incoming.data.bytes[2];
                VIN[5] = incoming.data.bytes[3];
                VIN[6] = incoming.data.bytes[4];
                VIN[7] = incoming.data.bytes[5];
                VIN[8] = incoming.data.bytes[6];
                VIN[9] = incoming.data.bytes[7];
            }
            if ((incoming.id == IDFilter) && (incoming.data.bytes[0] == 0x22))
            {
                VIN[10] = incoming.data.bytes[1];
                VIN[11] = incoming.data.bytes[2];
                VIN[12] = incoming.data.bytes[3];
                VIN[13] = incoming.data.bytes[4];
                VIN[14] = incoming.data.bytes[5];
                VIN[15] = incoming.data.bytes[6];
                VIN[16] = incoming.data.bytes[7];
                isWait = false;
            }
        }

    }
    
    // Create directory paths
    uint8_t j = 0;
    for (uint8_t i = 9; i < 17; i++)
    {
        fullDir[j] = VIN[i];
        PIDDir[j] = VIN[i];
        j++;
    }
    fullDir[8] = '/';
    PIDDir[8] = '/';
    Serial.println(fullDir);
    Serial.println(PIDDir);
    SDPrint.createDRIVE(fullDir);
    
    j = 0;
    for (uint8_t i = 9; i < 16; i++)
    {
        PIDDir[i] = PID[j];
        j++;
    }
    Serial.println(PIDDir);
    j = 0;
    for (uint8_t i = 9; i < 16; i++)
    {
        fullDir[i] = VINLOG[j];
        j++;
    }
    Serial.println(fullDir);

    // Write VIN to log
    SDPrint.writeFile(fullDir, "VIN: ");
    SDPrint.writeFile(fullDir, VIN);
    SDPrint.writeFileln(fullDir);
    SDPrint.deleteFile(PIDDir);
}

// Future function
String CANBus::getVIN()
{
    return vehicleVIN;
}

// Future function
char* CANBus::getFullDir()
{
    return PIDDir;
}


void CANBus::setNextPID(bool next)
{
    hasNextPID = next;
}


bool CANBus::getNextPID()
{
    return hasNextPID;
}

// Method used to manually get the ID and byte array
void CANBus::getMessage(test& a, int& b)
{
    CAN_FRAME incoming;
    if (Can0.available() > 0) {
        Can0.read(incoming);
        b = incoming.id;
        for (int count = 0; count < incoming.length; count++) {
            a[count] = incoming.data.bytes[count];
        }
    }
}


void CANBus::CANTraffic() {
    CAN_FRAME incoming;
    if (Can0.available() > 0) {
        Can0.read(incoming);
        Serial.print("ID: 0x");
        Serial.print(incoming.id, HEX);
        Serial.print(" Len: ");
        Serial.print(incoming.length);
        Serial.print(" Data: 0x");
        for (int count = 0; count < incoming.length; count++) {
            Serial.print(incoming.data.bytes[count], HEX);
            Serial.print(" ");
        }
        Serial.print("\r\n");
    }
}


void CANBus::PIDStream(uint16_t sendID, uint8_t PID)
{
    // Create object to save message
    CAN_FRAME incoming;

    // Frames to request VIN
    uint8_t PIDRequest[8] = { 0x02, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

    PIDRequest[2] = PID;

    // Waits to recieve all messages
    bool isWait = true;

    SDPrint.writeFileln(fullDir);
    SDPrint.writeFile(fullDir, "PID ID: ");
    SDPrint.writeFile(fullDir, PID, HEX);
    SDPrint.writeFileln(fullDir);
    // Send first frame requesting VIN
    for (int i = 0; i < 5; i++)
    {
        isWait = true;
    sendFrame(sendID, PIDRequest);
    delay(10);
        if (Can0.available() > 0) {
            Can0.read(incoming);
            switch (PID)
            {
            case PID_ENGINE_RPM:
                if (incoming.data.bytes[2] == PID_ENGINE_RPM) {
                    uint16_t rpm;
                    rpm = ((256 * incoming.data.bytes[3]) + incoming.data.bytes[4]) / 4; //formula 256*A+B/4
                    Serial.print(F("Engine RPM: "));
                    Serial.println(rpm, DEC);
                    SDPrint.writeFile(fullDir, "Engine RPM: ");
                    SDPrint.writeFile(fullDir, rpm, DEC);
                    SDPrint.writeFileln(fullDir);
                    isWait = false;
                }
            case PID_FUEL_LEVEL:
                if (incoming.data.bytes[2] == PID_FUEL_LEVEL) {
                    uint16_t lev;
                    lev = ((100 * incoming.data.bytes[1])) / 255; //formula 100*A/255
                    Serial.print(F("Fuel Level (%): "));
                    Serial.println(lev, DEC);
                    SDPrint.writeFile(fullDir, "Fuel Level (%): ");
                    SDPrint.writeFile(fullDir, lev, DEC);
                    SDPrint.writeFileln(fullDir);
                }
                break;
            case PID_THROTTLE_POSITION:
                if (incoming.data.bytes[2] == PID_THROTTLE_POSITION) {
                    uint16_t pos;
                    pos = ((100 * incoming.data.bytes[3])) / 255; //formula 100*A/255
                    Serial.print(F("Throttle (%): "));
                    Serial.println(pos, DEC);
                    SDPrint.writeFile(fullDir, "Throttle (%): ");
                    SDPrint.writeFile(fullDir, pos, DEC);
                    SDPrint.writeFileln(fullDir);
                    
                }
            case PID_VEHICLE_SPEED:
                if (incoming.data.bytes[2] == PID_VEHICLE_SPEED) {
                    uint16_t spd;
                    spd = ((incoming.data.bytes[3])) / 1.609344; //formula 100*A/255
                    Serial.print(F("MPH: "));
                    Serial.println(spd, DEC);
                    SDPrint.writeFile(fullDir, "MPH: ");
                    SDPrint.writeFile(fullDir, spd, DEC);
                    SDPrint.writeFileln(fullDir);
                }
                break;
            case PID_MAF_FLOW:
                if (incoming.data.bytes[2] == PID_MAF_FLOW) {
                    uint16_t flow;
                    flow = ((256 * incoming.data.bytes[3])+ incoming.data.bytes[4]) / 100; //formula 100*A/255
                    Serial.print(F("MAF: (gram/s) "));
                    Serial.println(flow, DEC);
                    SDPrint.writeFile(fullDir, "MAF: ");
                    SDPrint.writeFile(fullDir, flow, DEC);
                    SDPrint.writeFileln(fullDir);
                }
                break;
            }
            Serial.print(F("ID: 0x"));
            SDPrint.writeFile(fullDir, "ID: 0x");
            Serial.print(incoming.id, HEX);
            SDPrint.writeFile(fullDir, incoming.id, HEX);
            Serial.print(F(" Len: "));
            SDPrint.writeFile(fullDir, " Len: ");
            Serial.print(incoming.length);
            SDPrint.writeFile(fullDir, incoming.length, HEX);
            Serial.print(F(" Data: "));
            SDPrint.writeFile(fullDir, " Data: ");
            for (int count = 0; count < incoming.length; count++) {
                Serial.print(incoming.data.bytes[count], HEX);
                SDPrint.writeFile(fullDir, incoming.data.bytes[count], HEX);
                Serial.print(" ");
                SDPrint.writeFile(fullDir, " ");
            }
            Serial.print("\r\n");
            Serial.println("");
            SDPrint.writeFileln(fullDir);
        }
    delay(1000);
    }
}

int CANBus::PIDStreamGauge(uint16_t sendID, uint8_t PID)
{
    // Create object to save message
    CAN_FRAME incoming;

    // Frames to request VIN
    uint8_t PIDRequest[8] = { 0x02, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
    
    PIDRequest[2] = PID;

    // Send first frame requesting VIN
    for (int i = 0; i < 5; i++)
    {
        sendFrame(sendID, PIDRequest);
        delay(10);
        if (Can0.available() > 0) {
            Can0.read(incoming);
            switch (PID)
            {
            case PID_ENGINE_RPM:
                if (incoming.data.bytes[2] == PID_ENGINE_RPM) {
                    uint16_t rpm;
                    rpm = ((256 * incoming.data.bytes[3]) + incoming.data.bytes[4]) / 4; //formula 256*A+B/4
                    return rpm;
                }
            case PID_FUEL_LEVEL:
                if (incoming.data.bytes[2] == PID_FUEL_LEVEL) {
                    uint16_t level = ((100 * incoming.data.bytes[1])) / 255; //formula 100*A/255
                    return level;
                }
                break;
            case PID_THROTTLE_POSITION:
                if (incoming.data.bytes[2] == PID_THROTTLE_POSITION) {
                    uint16_t pos = ((100 * incoming.data.bytes[3])) / 255; //formula 100*A/255
                    return pos;
                }
            case PID_VEHICLE_SPEED:
                if (incoming.data.bytes[2] == PID_VEHICLE_SPEED) {
                    uint16_t speed = ((incoming.data.bytes[3])) / 1.609344; //formula 100*A/255
                    return speed;
                }
                break;
            case PID_MAF_FLOW:
                if (incoming.data.bytes[2] == PID_MAF_FLOW) {
                    uint16_t airFlow = ((256 * incoming.data.bytes[3]) + incoming.data.bytes[4]) / 100; //formula 100*A/255
                    return airFlow;
                }
                break;
            case PID_ENGINE_LOAD:
                if (incoming.data.bytes[2] == PID_ENGINE_LOAD) {
                    uint16_t engineLoad = (incoming.data.bytes[3]) / 2.55; //formula A/2.55
                    return engineLoad;
                }
                break;
            case PID_COOLANT_TEMP:
                if (incoming.data.bytes[2] == PID_COOLANT_TEMP) {
                    uint16_t coolantTemp = (((incoming.data.bytes[3]) - 40) * 1.8) + 32; //formula A - 40 for C, C * 1.8 + 32 = F
                    return coolantTemp;
                }
                break;
            }
        }
    }
    return -1;
}