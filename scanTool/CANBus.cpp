// CANBus manages the CAN bus hardware

#include "CANBus.h"
#include <due_can.h>
#include "variant.h"

// Checks a single bit of binary number
#define CHECK_BIT(var,pos) ((var) & (1<<(pos)))

#define VINLOG "LOG.txt"
#define PIDS "PIDS.txt"

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

    // Frames to request VIN
    unsigned char ReadVIN1st[8] = { 0x02, 0x09, 0x02, 0x55, 0x55, 0x55, 0x55, 0x55 };
    unsigned char ReadVIN_2nd[8] = { 0x30, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

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
    SDPrint.createDRIVE(fullDir);
    strcat(PIDDir, PIDS);
    strcat(fullDir, VINLOG);

    // Write VIN to log
    Serial.println(fullDir);
    SDPrint.writeFile(fullDir, "VIN: ");
    SDPrint.writeFile(fullDir, VIN);
    SDPrint.writeFileln(fullDir);
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