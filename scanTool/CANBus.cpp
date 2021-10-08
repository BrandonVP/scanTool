// CANBus manages the CAN bus hardware

#include "CANBus.h"
#include <due_can.h>
#include "variant.h"
#include "PIDS.h"
#include "common.h"

// Initialize CAN1 and set the baud rates here
void CANBus::startCAN0(uint32_t start, uint32_t end)
{
	Can0.begin(baud0);
	Can0.watchForRange(start, end);

	// Do not use extended frames
	CANOut.extended = false;

	// Message length
	CANOut.length = 8;
}

// Initialize CAN1 and set the baud rates here
void CANBus::startCAN1(uint32_t start, uint32_t end)
{
	Can1.begin(baud1);
	Can1.watchForRange(start, end);
}

// Set Can0 Filter and Mask
void CANBus::setFilterMask0(uint32_t filter, uint32_t mask)
{
	Can0.begin(baud0);
	Can0.watchFor(filter, mask);
}

// Set Can1 Filter and Mask
void CANBus::setFilterMask1(uint32_t filter, uint32_t mask)
{
	Can1.begin(baud1);
	Can1.watchFor(filter, mask);
}

// Set baud rate for CAN Bus
void CANBus::setBaud0(uint32_t newBaud)
{
	baud0 = newBaud;
	Can0.set_baudrate(baud0);
}

void CANBus::setBaud1(uint32_t newBaud)
{
	baud1 = newBaud;
	Can0.set_baudrate(baud1);
	Can1.set_baudrate(baud1);
}

uint32_t CANBus::findBaudRate0()
{
	return Can0.beginAutoSpeed();
}

uint32_t CANBus::findBaudRate1()
{
	return Can0.beginAutoSpeed();
}

void ECUtraffic(CAN_FRAME* incCAN0)
{
	char buffer[50];
	uint32_t temp = millis();
	sprintf(buffer, "%08d   %04X   %d   %02X  %02X  %02X  %02X  %02X  %02X  %02X  %02X\r\n", temp, incCAN0->id, incCAN0->length, incCAN0->data.bytes[0], incCAN0->data.bytes[1], incCAN0->data.bytes[2], incCAN0->data.bytes[3], incCAN0->data.bytes[4], incCAN0->data.bytes[5], incCAN0->data.bytes[6], incCAN0->data.bytes[7]);
	SerialUSB.print(buffer);
}

// Set baud rates for both CAN Bus
void CANBus::startPID()
{
	Can0.setRXFilter(0, 0x7E8, 0x7C8, false);
	Can0.setCallback(0, ECUtraffic);
}

// Get the current baud rate
uint32_t CANBus::getBaud0()
{
	return baud0;
}

uint32_t CANBus::getBaud1()
{
	return baud1;
}

// True if there are still PIDs left to scan 
void CANBus::setNextPID(bool next)
{
	hasNextPID = next;
}

// Used by function in main
bool CANBus::getNextPID()
{
	return hasNextPID;
}

bool CANBus::VINReady()
{
	return hasVIN;
}
// Return the last VIN scanned
String CANBus::getVIN()
{
	return vehicleVIN;
}

// Future function
char* CANBus::getFullDir()
{
	return PIDDir;
}

void CANBus::setIDCANOut(uint16_t id)
{
	CANOut.id = id;
}

uint16_t CANBus::getCANOutID()
{
	return CANOut.id;
}

void CANBus::setDataCANOut(uint8_t value, uint8_t position)
{
	CANOut.data.byte[position] = value;
}

uint8_t CANBus::getCANOutData(uint8_t position)
{
	return CANOut.data.bytes[position];
}

void CANBus::sendCANOut(uint8_t channel, bool serialOut)
{
	if (channel == 0)
	{
		Can0.sendFrame(CANOut);
	}
	if (channel == 1)
	{
		Can1.sendFrame(CANOut);
	}
	if (serialOut)
	{
		char buffer[50];
		uint32_t temp = millis();
		Can0.read(incCAN0);
		sprintf(buffer, "%08d   %04X   %d   %02X  %02X  %02X  %02X  %02X  %02X  %02X  %02X\r\n", temp, CANOut.id, CANOut.length, CANOut.data.bytes[0], CANOut.data.bytes[1], CANOut.data.bytes[2], CANOut.data.bytes[3], CANOut.data.bytes[4], CANOut.data.bytes[5], CANOut.data.bytes[6], CANOut.data.bytes[7]);
		SerialUSB.print(buffer);
	}
}

// Send out CAN Bus message
void CANBus::sendFrame(uint32_t id, byte* frame, uint8_t frameLength = 8, bool serialOut = false)
{
	// Outgoing message ID
	CANOut.id = id;

	// Do not use extended frames
	CANOut.extended = false;

	// Message length
	CANOut.length = frameLength;

	// Assign object to message array
	for (uint8_t i = 0; i < frameLength; i++)
	{
		CANOut.data.byte[i] = frame[i];
	}

	Can0.sendFrame(CANOut);

	if (serialOut)
	{
		char buffer[50];
		uint32_t temp = millis();
		Can0.read(incCAN0);
		sprintf(buffer, "%08d   %04X   %d   %02X  %02X  %02X  %02X  %02X  %02X  %02X  %02X\r\n", temp, CANOut.id, CANOut.length, CANOut.data.bytes[0], CANOut.data.bytes[1], CANOut.data.bytes[2], CANOut.data.bytes[3], CANOut.data.bytes[4], CANOut.data.bytes[5], CANOut.data.bytes[6], CANOut.data.bytes[7]);
		SerialUSB.print(buffer);
	}
}

// Find supported vehicle PIDS
void CANBus::getPIDList(uint8_t range, uint8_t bank)
{
	bool isWait = true;
	uint16_t txid = 0x7DF;
	uint16_t IDFilter = 0x7E8;

	// Message used to requrest range of PIDS
	byte frame[8] = { 0x02, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
	frame[2] = range;
	sendFrame(txid, frame);

	unsigned long timer = millis();

	while (isWait && (millis() - timer < 10000))
	{
		if (Can0.available() > 0) {
			Can0.read(incCAN0);
			if (incCAN0.id == IDFilter)
			{
				// End loop since message was recieved
				isWait = false;

				// Log PID message
				sdCard.writeFile(fullDir, "RX ID: ");
				sdCard.writeFile(fullDir, incCAN0.id, HEX);
				sdCard.writeFile(fullDir, " MSG: ");
				for (int count = 0; count < incCAN0.length; count++)
				{
					sdCard.writeFile(fullDir, incCAN0.data.bytes[count], HEX);
					sdCard.writeFile(fullDir, " ");
				}
				sdCard.writeFileln(fullDir);

				// Check to see if the vehicle supports more PIDS
				setNextPID(CHECK_BIT(incCAN0.data.byte[6], 0));

				// Write the values from the bank if the Corresponding bit matches
				int pos = 0;
				for (int i = 3; i < 7; i++)
				{
					for (int j = 7; j >= 0; j--)
					{
						if (CHECK_BIT(incCAN0.data.bytes[i], j))
						{
							sdCard.writeFile(PIDDir, "0x");
							sdCard.writeFile(PIDDir, PID_bank[bank][pos], HEX);
							sdCard.writeFileln(PIDDir);
						}
						pos++;
					}
				}
			}
		}
	}
	return;
}

// Get VIN, last argument gives option to save in SD Card
uint8_t CANBus::requestVIN(uint16_t state, bool saveSD)
{
	uint8_t ReadVIN1st[8] = { 0x02, 0x09, 0x02, 0x55, 0x55, 0x55, 0x55, 0x55 };
	uint8_t ReadVIN_2nd[8] = { 0x30, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

	switch (state)
	{
	case 0:
		setFilterMask0(0x7E8, 0x7FF);
		sendFrame(0x7E0, ReadVIN1st);
		return ++state;
		break;
	case 1:
		if (Can0.available() > 0)
		{
			Can0.read(incCAN0);
			if (incCAN0.data.bytes[0] == 0x10)
			{
				VIN[0] = incCAN0.data.bytes[5];
				VIN[1] = incCAN0.data.bytes[6];
				VIN[2] = incCAN0.data.bytes[7];

				// Send second frame to continue VIN request

				sendFrame(0x7E0, ReadVIN_2nd);
				return ++state;
			}
		}
		return state;
		break;
	case 2:
		if (Can0.available() > 0)
		{
			Can0.read(incCAN0);
			if (incCAN0.data.bytes[0] == 0x21)
			{
				VIN[3] = incCAN0.data.bytes[1];
				VIN[4] = incCAN0.data.bytes[2];
				VIN[5] = incCAN0.data.bytes[3];
				VIN[6] = incCAN0.data.bytes[4];
				VIN[7] = incCAN0.data.bytes[5];
				VIN[8] = incCAN0.data.bytes[6];
				VIN[9] = incCAN0.data.bytes[7];
				return ++state;
			}
		}
		return state;
		break;
	case 3:
		if (Can0.available() > 0)
		{
			Can0.read(incCAN0);
			if (incCAN0.data.bytes[0] == 0x22)
			{
				VIN[10] = incCAN0.data.bytes[1];
				VIN[11] = incCAN0.data.bytes[2];
				VIN[12] = incCAN0.data.bytes[3];
				VIN[13] = incCAN0.data.bytes[4];
				VIN[14] = incCAN0.data.bytes[5];
				VIN[15] = incCAN0.data.bytes[6];
				VIN[16] = incCAN0.data.bytes[7];
				vehicleVIN = String(VIN);
				hasVIN = true;
				can1.setFilterMask0(0x000, 0x7FF);
				return ++state;
			}
		}
		return state;
		break;
	case 4:
		if (saveSD)
		{
			char VINLOG[7] = { 'L', 'O', 'G', '.', 't', 'x', 't' };
			char PID[7] = { 'P', 'I', 'D', '.', 't', 'x', 't' };

			// Directory temp
			char temp[20];

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
			SerialUSB.println(fullDir);
			SerialUSB.println(PIDDir);
			sdCard.createDRIVE(fullDir);

			j = 0;
			for (uint8_t i = 9; i < 16; i++)
			{
				PIDDir[i] = PID[j];
				j++;
			}
			SerialUSB.println(PIDDir);
			j = 0;
			for (uint8_t i = 9; i < 16; i++)
			{
				fullDir[i] = VINLOG[j];
				j++;
			}
			SerialUSB.println(fullDir);

			// Write VIN to log
			sdCard.writeFile(fullDir, "VIN: ");
			sdCard.writeFile(fullDir, VIN);
			sdCard.writeFileln(fullDir);
			sdCard.deleteFile(PIDDir);
			return ++state;
		}
		return state;
		break;
	}
	return state;
}

//
int CANBus::PIDStream(uint16_t sendID, uint8_t PID, bool saveToSD)
{
	// Frames to request VIN
	uint8_t PIDRequest[8] = { 0x02, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

	PIDRequest[2] = PID;

	// Waits to recieve all messages
	bool isWait = true;

	if (saveToSD)
	{
		sdCard.writeFileln(fullDir);
		sdCard.writeFile(fullDir, "PID ID: ");
		sdCard.writeFile(fullDir, PID, HEX);
		sdCard.writeFileln(fullDir);
	}

	// Send first frame requesting VIN
	isWait = true;
	sendFrame(sendID, PIDRequest);
	delay(10);
	if (Can0.available() > 0) {
		Can0.read(incCAN0);
		switch (PID)
		{
		case PID_ENGINE_RPM:
			if (incCAN0.data.bytes[2] == PID_ENGINE_RPM) {
				uint16_t rpm;
				rpm = ((256 * incCAN0.data.bytes[3]) + incCAN0.data.bytes[4]) / 4; //formula 256*A+B/4
				//Serial.print(F("Engine RPM: "));
				//Serial.println(rpm, DEC);
				if (saveToSD)
				{
					sdCard.writeFile(fullDir, "Engine RPM: ");
					sdCard.writeFile(fullDir, rpm, DEC);
					sdCard.writeFileln(fullDir);
				}
				isWait = false;
				return rpm;
			}
		case PID_FUEL_LEVEL:
			if (incCAN0.data.bytes[2] == PID_FUEL_LEVEL) {
				uint16_t level;
				level = ((100 * incCAN0.data.bytes[1])) / 255; //formula 100*A/255
				//Serial.print(F("Fuel Level (%): "));
				//Serial.println(lev, DEC);
				if (saveToSD)
				{
					sdCard.writeFile(fullDir, "Fuel Level (%): ");
					sdCard.writeFile(fullDir, level, DEC);
					sdCard.writeFileln(fullDir);
				}
				return level;
			}
			break;
		case PID_THROTTLE_POSITION:
			if (incCAN0.data.bytes[2] == PID_THROTTLE_POSITION) {
				uint16_t position;
				position = ((100 * incCAN0.data.bytes[3])) / 255; //formula 100*A/255
				//Serial.print(F("Throttle (%): "));
				//Serial.println(pos, DEC);
				if (saveToSD)
				{
					sdCard.writeFile(fullDir, "Throttle (%): ");
					sdCard.writeFile(fullDir, position, DEC);
					sdCard.writeFileln(fullDir);
				}
				return position;
			}
		case PID_VEHICLE_SPEED:
			if (incCAN0.data.bytes[2] == PID_VEHICLE_SPEED) {
				uint16_t vehicleSpeed;
				vehicleSpeed = ((incCAN0.data.bytes[3])) / 1.609344; //formula 100*A/255
				//Serial.print(F("MPH: "));
				//Serial.println(spd, DEC);
				if (saveToSD)
				{
					sdCard.writeFile(fullDir, "MPH: ");
					sdCard.writeFile(fullDir, vehicleSpeed, DEC);
					sdCard.writeFileln(fullDir);
				}
				return vehicleSpeed;
			}
			break;
		case PID_MASS_AIR_FLOW:
			if (incCAN0.data.bytes[2] == PID_MASS_AIR_FLOW) {
				uint16_t airFlow;
				airFlow = ((256 * incCAN0.data.bytes[3]) + incCAN0.data.bytes[4]) / 100; //formula 100*A/255
				//Serial.print(F("MAF: (gram/s) "));
				//Serial.println(flow, DEC);
				if (saveToSD)
				{
					sdCard.writeFile(fullDir, "MAF: ");
					sdCard.writeFile(fullDir, airFlow, DEC);
					sdCard.writeFileln(fullDir);
				}
				return airFlow;
			}
			break;
		}

		if (saveToSD)
		{
			sdCard.writeFile(fullDir, "ID: 0x");
			sdCard.writeFile(fullDir, incCAN0.id, HEX);
			sdCard.writeFile(fullDir, " Len: ");
			sdCard.writeFile(fullDir, incCAN0.length, HEX);
			sdCard.writeFile(fullDir, " Data: ");
			//Serial.print(F("ID: 0x"));
			//Serial.print(incCAN0.id, HEX);
			//Serial.print(F(" Len: "));
			//Serial.print(incCAN0.length);
			//Serial.print(F(" Data: "));

			for (int count = 0; count < incCAN0.length; count++)
			{
				Serial.print(incCAN0.data.bytes[count], HEX);
				sdCard.writeFile(fullDir, incCAN0.data.bytes[count], HEX);
				Serial.print(" ");
				sdCard.writeFile(fullDir, " ");
			}
			//Serial.print("\r\n");
			//Serial.println("");
			sdCard.writeFileln(fullDir);
		}
	}
}

//
int CANBus::PIDStreamGauge(uint16_t sendID, uint8_t PID)
{
	// Frames to request VIN
	uint8_t PIDRequest[8] = { 0x02, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

	PIDRequest[2] = PID;

	bool isWait = true;
	unsigned long timer = millis();

	// Send first frame requesting VIN
	for (int i = 0; i < 5; i++)
	{
		sendFrame(sendID, PIDRequest);

		while (millis() - timer < 100)
		{
			if (Can0.available() > 0) {
				Can0.read(incCAN0);
				switch (PID)
				{
				case PID_ENGINE_RPM:
					if (incCAN0.data.bytes[2] == PID_ENGINE_RPM) {
						uint16_t rpm;
						rpm = ((256 * incCAN0.data.bytes[3]) + incCAN0.data.bytes[4]) / 4; //formula 256*A+B/4
						return rpm;
					}
				case PID_FUEL_LEVEL:
					if (incCAN0.data.bytes[2] == PID_FUEL_LEVEL) {
						uint16_t level = ((100 * incCAN0.data.bytes[1])) / 255; //formula 100*A/255
						return level;
					}
					break;
				case PID_THROTTLE_POSITION:
					if (incCAN0.data.bytes[2] == PID_THROTTLE_POSITION) {
						uint16_t pos = ((100 * incCAN0.data.bytes[3])) / 255; //formula 100*A/255
						return pos;
					}
				case PID_VEHICLE_SPEED:
					if (incCAN0.data.bytes[2] == PID_VEHICLE_SPEED) {
						uint16_t speed = ((incCAN0.data.bytes[3])) / 1.609344; //formula 100*A/255
						return speed;
					}
					break;
				case PID_MASS_AIR_FLOW:
					if (incCAN0.data.bytes[2] == PID_MASS_AIR_FLOW) {
						uint16_t airFlow = ((256 * incCAN0.data.bytes[3]) + incCAN0.data.bytes[4]) / 100; //formula 100*A/255
						return airFlow;
					}
					break;
				case PID_ENGINE_LOAD:
					if (incCAN0.data.bytes[2] == PID_ENGINE_LOAD) {
						uint16_t engineLoad = (incCAN0.data.bytes[3]) / 2.55; //formula A/2.55
						return engineLoad;
					}
					break;
				case PID_COOLANT_TEMP:
					if (incCAN0.data.bytes[2] == PID_COOLANT_TEMP) {
						uint16_t coolantTemp = (((incCAN0.data.bytes[3]) - 40) * 1.8) + 32; //formula A - 40 for C, C * 1.8 + 32 = F
						return coolantTemp;
					}
					break;
				}
			}
		}
	}
	return -1;
}

// Send CAN Bus traffic from channel 1 or 2 to LCD
bool CANBus::LCDOutCAN(buff& msg, uint8_t& len, uint32_t& id, uint8_t config)
{
	if (config == 1 && Can0.get_rx_buff(incCAN0))
	{
		id = incCAN0.id;
		len = incCAN0.length;
		for (int count = 0; count < incCAN0.length; count++) {
			msg[count] = incCAN0.data.bytes[count];
		}
		return true;
	}
	else if (config == 2 && Can1.get_rx_buff(incCAN1))
	{
		id = incCAN1.id;
		len = incCAN1.length;
		for (int count = 0; count < incCAN1.length; count++) {
			msg[count] = incCAN1.data.bytes[count];
		}
		return true;
	}
	else if (config == 3)
	{
		if (Can0.get_rx_buff(incCAN0))
		{
			id = incCAN0.id;
			len = incCAN0.length;
			for (int count = 0; count < incCAN0.length; count++) {
				msg[count] = incCAN0.data.bytes[count];
			}
			return true;
		}
		if (Can1.get_rx_buff(incCAN1))
		{
			id = incCAN1.id;
			len = incCAN1.length;
			for (int count = 0; count < incCAN1.length; count++) {
				msg[count] = incCAN1.data.bytes[count];
			}
			return true;
		}
	}
	else if (config == 4)
	{
		if (Can0.get_rx_buff(incCAN0))
		{
			Can1.sendFrame(incCAN0);
		}
		if (Can1.get_rx_buff(incCAN1))
		{
			id = incCAN1.id;
			len = incCAN1.length;
			for (int count = 0; count < incCAN1.length; count++) {
				msg[count] = incCAN1.data.bytes[count];
			}
			Can0.sendFrame(incCAN1);
			return true;
		}
	}
	else if (config == 5)
	{
		if (Can0.get_rx_buff(incCAN0))
		{
			id = incCAN0.id;
			len = incCAN0.length;
			for (int count = 0; count < incCAN0.length; count++) {
				msg[count] = incCAN0.data.bytes[count];
			}
			return true;
		}
		if (Can1.get_rx_buff(incCAN1))
		{
			id = incCAN1.id;
			len = incCAN1.length;
			for (int count = 0; count < incCAN1.length; count++) {
				msg[count] = incCAN1.data.bytes[count];
			}
			return true;
		}
	}
	return false;
}

// Displays CAN traffic on Serial out
bool CANBus::SerialOutCAN(uint8_t config)
{
	// Display CAN0
	if (config == 1 && Can0.get_rx_buff(incCAN0))
	{
		sprintf(buffer, "%08d   %04X   %d   %02X  %02X  %02X  %02X  %02X  %02X  %02X  %02X\r\n", millis(), incCAN0.id, incCAN0.length, incCAN0.data.bytes[0], incCAN0.data.bytes[1], incCAN0.data.bytes[2], incCAN0.data.bytes[3], incCAN0.data.bytes[4], incCAN0.data.bytes[5], incCAN0.data.bytes[6], incCAN0.data.bytes[7]);
		SerialUSB.print(buffer);
	}
	// Display CAN1
	else if (config == 2 && Can1.get_rx_buff(incCAN1))
	{
		sprintf(buffer, "%08d   %04X   %d   %02X  %02X  %02X  %02X  %02X  %02X  %02X  %02X\r\n", millis(), incCAN1.id, incCAN1.length, incCAN1.data.bytes[0], incCAN1.data.bytes[1], incCAN1.data.bytes[2], incCAN1.data.bytes[3], incCAN1.data.bytes[4], incCAN1.data.bytes[5], incCAN1.data.bytes[6], incCAN1.data.bytes[7]);
		SerialUSB.print(buffer);
	}
	// Display CAN0 & CAN1
	else if (config == 3)
	{
		if (Can0.get_rx_buff(incCAN0))
		{
			sprintf(buffer, "%08d   %04X   %d   %02X  %02X  %02X  %02X  %02X  %02X  %02X  %02X\r\n", millis(), incCAN0.id, incCAN0.length, incCAN0.data.bytes[0], incCAN0.data.bytes[1], incCAN0.data.bytes[2], incCAN0.data.bytes[3], incCAN0.data.bytes[4], incCAN0.data.bytes[5], incCAN0.data.bytes[6], incCAN0.data.bytes[7]);
			SerialUSB.print(buffer);
		}
		if (Can1.get_rx_buff(incCAN1))
		{
			sprintf(buffer, "%08d   %04X   %d   %02X  %02X  %02X  %02X  %02X  %02X  %02X  %02X\r\n", millis(), incCAN1.id, incCAN1.length, incCAN1.data.bytes[0], incCAN1.data.bytes[1], incCAN1.data.bytes[2], incCAN1.data.bytes[3], incCAN1.data.bytes[4], incCAN1.data.bytes[5], incCAN1.data.bytes[6], incCAN1.data.bytes[7]);
			SerialUSB.print(buffer);
		}
	}
	// Forward traffic between CAN0-CAN1 and display CAN0
	else if (config == 4)
	{
		if (Can0.get_rx_buff(incCAN0))
		{
			Can1.sendFrame(incCAN0);
		}
		if (Can1.get_rx_buff(incCAN1))
		{
			sprintf(buffer, "%08d   %04X   %d   %02X  %02X  %02X  %02X  %02X  %02X  %02X  %02X\r\n", millis(), incCAN1.id, incCAN1.length, incCAN1.data.bytes[0], incCAN1.data.bytes[1], incCAN1.data.bytes[2], incCAN1.data.bytes[3], incCAN1.data.bytes[4], incCAN1.data.bytes[5], incCAN1.data.bytes[6], incCAN1.data.bytes[7]);
			SerialUSB.print(buffer);
			Can0.sendFrame(incCAN1);
		}
	}
	// Forward traffic between CAN0-CAN1 and display CAN0 & CAN1
	else if (config == 5)
	{
		if (Can0.get_rx_buff(incCAN0))
		{
			sprintf(buffer, "%08d   %04X   %d   %02X  %02X  %02X  %02X  %02X  %02X  %02X  %02X\r\n", millis(), incCAN0.id, incCAN0.length, incCAN0.data.bytes[0], incCAN0.data.bytes[1], incCAN0.data.bytes[2], incCAN0.data.bytes[3], incCAN0.data.bytes[4], incCAN0.data.bytes[5], incCAN0.data.bytes[6], incCAN0.data.bytes[7]);
			SerialUSB.print(buffer);
			Can1.sendFrame(incCAN0);
		}
		if (Can1.get_rx_buff(incCAN1))
		{
			sprintf(buffer, "%08d   %04X   %d   %02X  %02X  %02X  %02X  %02X  %02X  %02X  %02X\r\n", millis(), incCAN1.id, incCAN1.length, incCAN1.data.bytes[0], incCAN1.data.bytes[1], incCAN1.data.bytes[2], incCAN1.data.bytes[3], incCAN1.data.bytes[4], incCAN1.data.bytes[5], incCAN1.data.bytes[6], incCAN1.data.bytes[7]);
			SerialUSB.print(buffer);
			Can0.sendFrame(incCAN1);
		}
	}
	return true;
}
