// CANBus manages the CAN bus hardware

#include "CANBus.h"
#include <due_can.h>
#include "variant.h"
//#include "definitions.h"

// Checks single bit of binary number
#define CHECK_BIT(var,pos) ((var) & (1<<(pos)))

#define PIDLIST "LOG.txt"
#define PIDS "PIDS.txt"

CANBus::CANBus()
{

}

void CANBus::startCAN()
{
    // Initialize CAN1 and set the proper baud rates here
    Can0.begin(CAN_BPS_500K);
    Can0.watchForRange(0x7E0, 0x7EF);
    return;
}

// CAN Bus send message
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

    // Send object out
    /*
    SDPrint.writeFile(PIDLIST, "TX ID: ");
    SDPrint.writeFile(PIDLIST, sending.id);
    SDPrint.writeFile(PIDLIST, " MSG: ");
    for (int count = 0; count < sending.length; count++)
    {
        SDPrint.writeFile(PIDLIST, sending.data.bytes[count], HEX);
        SDPrint.writeFile(PIDLIST, " ");
    }
    SDPrint.writeFileln(PIDLIST);
    */
    Can0.sendFrame(sending);
    return;
}

// Method used for CAN recording
bool CANBus::getFrame(uint16_t IDFilter)
{
    // Create object to save message
    CAN_FRAME incoming;

    // If buffer inbox has a message
    if (Can0.available() > 0) {
        Can0.read(incoming);
        if (incoming.id == IDFilter)
        {   
            rxID = incoming.id;
            SDPrint.writeFile(fullDir, "RX ID: ");
            SDPrint.writeFile(fullDir, incoming.id, HEX);
            SDPrint.writeFile(fullDir, " MSG: ");
            for (int count = 0; count < incoming.length; count++) 
            {
                SDPrint.writeFile(fullDir, incoming.data.bytes[count], HEX);
                SDPrint.writeFile(fullDir, " ");
            }
            SDPrint.writeFileln(fullDir);
            bool hasNext = CHECK_BIT(incoming.data.byte[6], 0);
            setNextPID(hasNext);
            int pos = 0;
            int bank = 0;
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

            return false;
        }
    }
    return true;
}

//void CANBus::VIN()
//{
//
//}

void CANBus::findVIN(uint16_t IDFilter)
{
    // Create object to save message
    CAN_FRAME incoming;
    bool isWait = true;
    memset(fullDir, 0, sizeof(fullDir));

    // If buffer inbox has a message
    if (Can0.available() > 0) {
        Can0.read(incoming);
        while(isWait)
        { 
            if (incoming.id == IDFilter)
            {
                rxID = incoming.id;
                SDPrint.writeFile(PIDLIST, "RX ID: ");
                SDPrint.writeFile(PIDLIST, incoming.id, HEX);
                SDPrint.writeFile(PIDLIST, " MSG: ");
                for (int count = 0; count < incoming.length; count++)
                {
                    SDPrint.writeFile(PIDLIST, incoming.data.bytes[count], HEX);
                    SDPrint.writeFile(PIDLIST, " ");
                }
                SDPrint.writeFileln(PIDLIST);
                VIN[0] = incoming.data.bytes[5];
                VIN[1] = incoming.data.bytes[6];
                VIN[2] = incoming.data.bytes[7];
                isWait = false;
            }
        }
    }
    isWait = true;
    if (Can0.available() > 0) {
        Can0.read(incoming);
        while (isWait)
        {
            if (incoming.id == IDFilter)
            {
                rxID = incoming.id;
                SDPrint.writeFile(PIDLIST, "RX ID: ");
                SDPrint.writeFile(PIDLIST, incoming.id, HEX);
                SDPrint.writeFile(PIDLIST, " MSG: ");
                for (int count = 0; count < incoming.length; count++)
                {
                    SDPrint.writeFile(PIDLIST, incoming.data.bytes[count], HEX);
                    SDPrint.writeFile(PIDLIST, " ");
                }
                SDPrint.writeFileln(PIDLIST);
                VIN[3] = incoming.data.bytes[1];
                VIN[4] = incoming.data.bytes[2];
                VIN[5] = incoming.data.bytes[3];
                VIN[6] = incoming.data.bytes[4];
                VIN[7] = incoming.data.bytes[5];
                VIN[8] = incoming.data.bytes[6];
                VIN[9] = incoming.data.bytes[7];
            }
            isWait = false;
        }

    }
    isWait = true;
    if (Can0.available() > 0) {
        Can0.read(incoming);
        while (isWait)
        {
            if (incoming.id == IDFilter)
            {
                rxID = incoming.id;
                SDPrint.writeFile(PIDLIST, "RX ID: ");
                SDPrint.writeFile(PIDLIST, incoming.id, HEX);
                SDPrint.writeFile(PIDLIST, " MSG: ");
                for (int count = 0; count < incoming.length; count++)
                {
                    SDPrint.writeFile(PIDLIST, incoming.data.bytes[count], HEX);
                    SDPrint.writeFile(PIDLIST, " ");
                }
                SDPrint.writeFileln(PIDLIST);
                VIN[10] = incoming.data.bytes[1];
                VIN[11] = incoming.data.bytes[2];
                VIN[12] = incoming.data.bytes[3];
                VIN[13] = incoming.data.bytes[4];
                VIN[14] = incoming.data.bytes[5];
                VIN[15] = incoming.data.bytes[6];
                VIN[16] = incoming.data.bytes[7];

            }
            isWait = false;
        }
    }
    uint8_t j = 0;
    for (uint8_t i = 9; i < 17; i++)
    {
        fullDir[j] = VIN[i]; 
        PIDDir[j] = VIN[i];
        j++;
    }
    fullDir[8] = '/';
    PIDDir[8] = '/';
    SDPrint.createDRIVE(fullDir);
    strcat(PIDDir, PIDS);
    strcat(fullDir, PIDLIST); 
    Serial.println(fullDir);
    SDPrint.writeFile(fullDir, "VIN: ");
    SDPrint.writeFile(fullDir, VIN);
    SDPrint.writeFile(PIDDir, "VIN: ");
    SDPrint.writeFile(PIDDir, VIN);
    SDPrint.writeFileln(PIDDir);
    SDPrint.writeFile(PIDDir, "Supported PIDS");
    SDPrint.writeFileln(PIDDir);
    return;
}

String CANBus::getVIN()
{
    return vehicleVIN;
}

char* CANBus::getFullDir()
{
    return fullDir;
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

uint16_t CANBus::getrxID()
{
    return rxID;
}
//MOVE TO METHODS
/*
CAN_FRAME incoming;
static unsigned long lastTime = 0;

if (Can0.available() > 0) {
    Can0.read(incoming);
    test = incoming.data.byte[0];
    sendData(test);


    SD.begin(CSPIN);        //SD Card is initialized
    //SD.remove("Test2.txt"); //remove any existing file with this name
    myFile = SD.open("Test2.txt", FILE_WRITE);  //file created and opened for writing

    if (myFile)        //file has really been opened
    {
        myFile.print("ID: ");
        myFile.print(incoming.id);
        myFile.print(" MSG:");
        for (int count = 0; count < incoming.length; count++) {
            myFile.print(incoming.data.bytes[count], HEX);
            myFile.print(" ");
        }
        myFile.println();
        myFile.close();

    }
}
*/