// CANBus manages the CAN bus hardware

#include "CANBus.h"
#include <due_can.h>
#include "variant.h"
#include "PIDS.h"
#include "common.h"

// Initialize CAN1 and set the baud rates here
void CANBus::startCAN0(uint32_t start, uint32_t end)
{
	Can0.begin(getBaud0());
	Can0.watchForRange(start, end);

	// Do not use extended frames
	CANOut.extended = false;

	// Message length
	CANOut.length = 8;
}

// Initialize CAN1 and set the baud rates here
void CANBus::startCAN1(uint32_t start, uint32_t end)
{
	Can1.begin(getBaud1());
	Can1.watchForRange(start, end);
}

// Set Can0 Filter and Mask
void CANBus::setFilterMask0(uint32_t filter, uint32_t mask)
{
	for (uint8_t i = 0; i < 7; i++)
	{
		Can0.setRXFilter(i, filter, mask, false);
	}
}

// Set Can1 Filter and Mask
void CANBus::setFilterMask1(uint32_t filter, uint32_t mask)
{
	for (uint8_t i = 0; i < 7; i++)
	{
		Can1.setRXFilter(i, filter, mask, false);
	}
}

// Set baud rate for CAN Bus
void CANBus::setBaud0(uint32_t newBaud)
{
	Can0.set_baudrate(newBaud);
}

void CANBus::setBaud1(uint32_t newBaud)
{
	Can1.set_baudrate(newBaud);
}

// Get the current baud rate
uint32_t CANBus::getBaud0()
{
	return Can0.getBusSpeed();
}

uint32_t CANBus::getBaud1()
{
	return Can1.getBusSpeed();
}

uint32_t CANBus::findBaudRate0()
{
	return Can0.beginAutoSpeed();
}

uint32_t CANBus::findBaudRate1()
{
	return Can1.beginAutoSpeed();
}

// PID callback
void ECUtraffic(CAN_FRAME* incCAN0)
{
	char buffer[50];
	uint32_t temp = millis();
	sprintf(buffer, "%08d   %04X   %d   %02X  %02X  %02X  %02X  %02X  %02X  %02X  %02X\r\n", temp, incCAN0->id, incCAN0->length, incCAN0->data.bytes[0], incCAN0->data.bytes[1], incCAN0->data.bytes[2], incCAN0->data.bytes[3], incCAN0->data.bytes[4], incCAN0->data.bytes[5], incCAN0->data.bytes[6], incCAN0->data.bytes[7]);
	SERIAL_CAPTURE(buffer);
}

// Set baud rates for both CAN Bus
void CANBus::startPID()
{
	Can0.setRXFilter(0, 0x7E8, 0x7C8, false);
	Can0.setCallback(0, ECUtraffic);
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

void CANBus::incCaptureFile()
{
	captureNumber++;
	capture_filename[4] = captureNumber;
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
	if (channel == 2)
	{
		Serial3.write(0xFE);
		Serial3.write(0x09);
		Serial3.write((CANOut.id >> 0) & 0xFF);
		Serial3.write((CANOut.id >> 8) & 0xFF);
		for (uint8_t i = 0; i < 8; i++)
		{
			Serial3.write(CANOut.data.bytes[i]);
		}
		Serial3.write(0xFD);
	}
	if (serialOut)
	{
		char buffer[50];
		uint32_t temp = millis();
		sprintf(buffer, "%08d   %04X   %d   %02X  %02X  %02X  %02X  %02X  %02X  %02X  %02X\r\n", temp, CANOut.id, CANOut.length, CANOut.data.bytes[0], CANOut.data.bytes[1], CANOut.data.bytes[2], CANOut.data.bytes[3], CANOut.data.bytes[4], CANOut.data.bytes[5], CANOut.data.bytes[6], CANOut.data.bytes[7]);
		SERIAL_CAPTURE(buffer);
	}
}

void CANBus::sendCANOut(uint8_t channel, CAN_FRAME CANBus, bool serialOut)
{
	if (channel == 0)
	{
		Can0.sendFrame(CANBus);
	}
	if (channel == 1)
	{
		Can1.sendFrame(CANBus);
	}
	if (channel == 2)
	{
		Serial3.write(0xFE);
		Serial3.write(0x09);
		Serial3.write((CANBus.id >> 0) & 0xFF);
		Serial3.write((CANBus.id >> 8) & 0xFF);
		for (uint8_t i = 0; i < 8; i++)
		{
			Serial3.write(CANBus.data.bytes[i]);
		}
		Serial3.write(0xFD);
	}
	if (serialOut)
	{
		char buffer[50];
		uint32_t temp = millis();
		sprintf(buffer, "%08d   %04X   %d   %02X  %02X  %02X  %02X  %02X  %02X  %02X  %02X\r\n", temp, CANBus.id, CANBus.length, CANBus.data.bytes[0], CANBus.data.bytes[1], CANBus.data.bytes[2], CANBus.data.bytes[3], CANBus.data.bytes[4], CANBus.data.bytes[5], CANBus.data.bytes[6], CANBus.data.bytes[7]);
		SERIAL_CAPTURE(buffer);
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
		SERIAL_CAPTURE(buffer);
	}
}

// Find supported vehicle PIDS
uint8_t CANBus::getPIDList(uint8_t state, uint8_t range, uint8_t bank)
{
	// Message used to requrest range of PIDS
	byte frame[8] = { 0x02, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
	frame[2] = range;

	switch (state)
	{
	case 1:
		setFilterMask0(0x0, 0x0);
		sendFrame(PID_LIST_TX, frame);
		return 2;
		break;
	case 2:
		if (Can0.get_rx_buff(incCAN0) && incCAN0.id == ECU_RX)
		{
			// Log PID message
			sprintf(buffer, "%08d   %04X   %d   %02X  %02X  %02X  %02X  %02X  %02X  %02X  %02X\r\n", millis(), incCAN0.id, incCAN0.length, incCAN0.data.bytes[0], incCAN0.data.bytes[1], incCAN0.data.bytes[2], incCAN0.data.bytes[3], incCAN0.data.bytes[4], incCAN0.data.bytes[5], incCAN0.data.bytes[6], incCAN0.data.bytes[7]);

			sdCard.writeFile(fullDir, buffer);
			// Write the values from the bank if the Corresponding bit matches
			uint16_t pos = 0;
			for (uint8_t i = 3; i < 7; i++)
			{
				for (int8_t j = 7; j >= 0; j--)
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
			(CHECK_BIT(incCAN0.data.byte[6], 0)) ? state = 3 : state = 4;
		}
		break;
	}
	return state;
}

// Get VIN, last argument gives option to save in SD Card
uint8_t CANBus::requestVIN(uint16_t state, bool saveSD)
{
	uint8_t ReadVIN1st[8] = { 0x02, 0x09, 0x02, 0x55, 0x55, 0x55, 0x55, 0x55 };
	uint8_t ReadVIN_2nd[8] = { 0x30, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

	switch (state)
	{
	case 0:
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
uint8_t CANBus::PIDStream(uint8_t &value, bool saveToSD)
{
	// Frames to request VIN
	uint8_t PIDRequest[8] = { 0x02, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

	//PIDRequest[2] = PID;

	// Waits to recieve all messages
	bool isWait = true;
	uint8_t PID = 0;
	/*
	if (saveToSD)
	{
		sdCard.writeFileln(fullDir);
		sdCard.writeFile(fullDir, "PID ID: ");
		sdCard.writeFile(fullDir, PID, HEX);
		sdCard.writeFileln(fullDir);
	}
	*/
	

	// Send first frame requesting VIN
	isWait = true;
	//sendFrame(sendID, PIDRequest);
	//delay(10);
	if (Can0.available() > 0) {
		Can0.read(incCAN0);
		PID = incCAN0.data.bytes[2];
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
				value = rpm;
				return PID;
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
				value = level;
				return PID;
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
				value = position;
				return PID;
			}
		case PID_VEHICLE_SPEED:
			if (incCAN0.data.bytes[2] == PID_VEHICLE_SPEED) {
				uint16_t vehicleSpeed;
				vehicleSpeed = ((incCAN0.data.bytes[3])) / 1.609344; //formula 100*A/255
				//SerialUSB.print(F("MPH: "));
				//SerialUSB.println(vehicleSpeed, DEC);
				if (saveToSD)
				{
					sdCard.writeFile(fullDir, "MPH: ");
					sdCard.writeFile(fullDir, vehicleSpeed, DEC);
					sdCard.writeFileln(fullDir);
				}
				value = vehicleSpeed;
				return PID;
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
				value = airFlow;
				return PID;
			}
			break;
		case PID_ENGINE_LOAD:
			if (incCAN0.data.bytes[2] == PID_ENGINE_LOAD) 
			{
				uint16_t engineLoad = (incCAN0.data.bytes[3]) / 2.55; //formula A/2.55
				value = engineLoad;
				return PID;
			}
			break;
		case PID_COOLANT_TEMP:
			if (incCAN0.data.bytes[2] == PID_COOLANT_TEMP) 
			{
				uint16_t coolantTemp = (((incCAN0.data.bytes[3]) - 40) * 1.8) + 32; //formula A - 40 for C, C * 1.8 + 32 = F
				value = coolantTemp;
				return PID;
			}
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
	return 0;
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
	else if (config == 6)
	{
		if (Serial3.available() > 0)
		{
			uint8_t recByte = Serial3.read();
			//SerialUSB.println(recByte, 16);
			switch (state)
			{
			case START_BYTE:
				if (recByte == STARTING_BYTE)
				{
					state = PACKET_LENGTH;
					return false;
				}
				break;
			case PACKET_LENGTH:
				state = CAN_BUS_ID1;
				if (recByte == PACKET_SIZE)
				{
					packetIndex = 0;
					return false;
				}
				else
				{
					// Bad packet
					state = START_BYTE;
				}
				break;
			case CAN_BUS_ID1:
				incWIFI.id = recByte;
				state = CAN_BUS_ID2;
				break;
			case CAN_BUS_ID2:
				incWIFI.id += (recByte << 8);
				state = CAN_BUS_DATA;
				break;
			case CAN_BUS_DATA:
				incWIFI.data.bytes[packetIndex] = recByte;
				packetIndex++;
				if (packetIndex == PACKET_SIZE - 1)
				{
					state = END_BYTE;
				}
				break;
			case END_BYTE:
				if (recByte == ENDING_BYTE)
				{
					state = START_BYTE;
					id = incWIFI.id;
					len = 8;
					for (int count = 0; count < incCAN0.length; count++) {
						msg[count] = incWIFI.data.bytes[count];
					}
					//sprintf(buffer, "%08d   %04X   %d   %02X  %02X  %02X  %02X  %02X  %02X  %02X  %02X\r\n", millis(), incWIFI.id, incWIFI.length, incWIFI.data.bytes[0], incWIFI.data.bytes[1], incWIFI.data.bytes[2], incWIFI.data.bytes[3], incWIFI.data.bytes[4], incWIFI.data.bytes[5], incWIFI.data.bytes[6], incWIFI.data.bytes[7]);
					//SERIAL_CAPTURE(buffer);
					return true;
				}
				else
				{
					// packet failed restart
					state = START_BYTE;
				}
				break;
			}
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
		SERIAL_CAPTURE(buffer);
	}
	// Display CAN1
	else if (config == 2 && Can1.get_rx_buff(incCAN1))
	{
		sprintf(buffer, "%08d   %04X   %d   %02X  %02X  %02X  %02X  %02X  %02X  %02X  %02X\r\n", millis(), incCAN1.id, incCAN1.length, incCAN1.data.bytes[0], incCAN1.data.bytes[1], incCAN1.data.bytes[2], incCAN1.data.bytes[3], incCAN1.data.bytes[4], incCAN1.data.bytes[5], incCAN1.data.bytes[6], incCAN1.data.bytes[7]);
		SERIAL_CAPTURE(buffer);
	}
	// Display CAN0 & CAN1
	else if (config == 3)
	{
		if (Can0.get_rx_buff(incCAN0))
		{
			sprintf(buffer, "%08d   %04X   %d   %02X  %02X  %02X  %02X  %02X  %02X  %02X  %02X\r\n", millis(), incCAN0.id, incCAN0.length, incCAN0.data.bytes[0], incCAN0.data.bytes[1], incCAN0.data.bytes[2], incCAN0.data.bytes[3], incCAN0.data.bytes[4], incCAN0.data.bytes[5], incCAN0.data.bytes[6], incCAN0.data.bytes[7]);
			SERIAL_CAPTURE(buffer);
		}
		if (Can1.get_rx_buff(incCAN1))
		{
			sprintf(buffer, "%08d   %04X   %d   %02X  %02X  %02X  %02X  %02X  %02X  %02X  %02X\r\n", millis(), incCAN1.id, incCAN1.length, incCAN1.data.bytes[0], incCAN1.data.bytes[1], incCAN1.data.bytes[2], incCAN1.data.bytes[3], incCAN1.data.bytes[4], incCAN1.data.bytes[5], incCAN1.data.bytes[6], incCAN1.data.bytes[7]);
			SERIAL_CAPTURE(buffer);
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
			SERIAL_CAPTURE(buffer);
			Can0.sendFrame(incCAN1);
		}
	}
	// Forward traffic between CAN0-CAN1 and display CAN0 & CAN1
	else if (config == 5)
	{
		if (Can0.get_rx_buff(incCAN0))
		{
			sprintf(buffer, "%08d   %04X   %d   %02X  %02X  %02X  %02X  %02X  %02X  %02X  %02X\r\n", millis(), incCAN0.id, incCAN0.length, incCAN0.data.bytes[0], incCAN0.data.bytes[1], incCAN0.data.bytes[2], incCAN0.data.bytes[3], incCAN0.data.bytes[4], incCAN0.data.bytes[5], incCAN0.data.bytes[6], incCAN0.data.bytes[7]);
			SERIAL_CAPTURE(buffer);
			Can1.sendFrame(incCAN0);
		}
		if (Can1.get_rx_buff(incCAN1))
		{
			sprintf(buffer, "%08d   %04X   %d   %02X  %02X  %02X  %02X  %02X  %02X  %02X  %02X\r\n", millis(), incCAN1.id, incCAN1.length, incCAN1.data.bytes[0], incCAN1.data.bytes[1], incCAN1.data.bytes[2], incCAN1.data.bytes[3], incCAN1.data.bytes[4], incCAN1.data.bytes[5], incCAN1.data.bytes[6], incCAN1.data.bytes[7]);
			SERIAL_CAPTURE(buffer);
			Can0.sendFrame(incCAN1);
		}
	}
	else if (config == 6)
	{
		if (Serial3.available() > 0)
		{
			uint8_t recByte = Serial3.read();
			//SerialUSB.println(recByte, 16);
			switch (state)
			{
			case START_BYTE:
				if (recByte == STARTING_BYTE)
				{
					state = PACKET_LENGTH;
					return false;
				}
				break;
			case PACKET_LENGTH:
				state = CAN_BUS_ID1;
				if (recByte == PACKET_SIZE)
				{
					packetIndex = 0;
					return false;
				}
				else
				{
					// Bad packet
					state = START_BYTE;
				}
				break;
			case CAN_BUS_ID1:
				incWIFI.id = recByte;
				state = CAN_BUS_ID2;
				break;
			case CAN_BUS_ID2:
				incWIFI.id += (recByte << 8);
				state = CAN_BUS_DATA;
				break;
			case CAN_BUS_DATA:
				incWIFI.data.bytes[packetIndex] = recByte;
				packetIndex++;
				if (packetIndex == PACKET_SIZE - 1)
				{
					state = END_BYTE;
				}
				break;
			case END_BYTE:
				if (recByte == ENDING_BYTE)
				{
					state = START_BYTE;
					sprintf(buffer, "%08d   %04X   %d   %02X  %02X  %02X  %02X  %02X  %02X  %02X  %02X\r\n", millis(), incWIFI.id, incWIFI.length, incWIFI.data.bytes[0], incWIFI.data.bytes[1], incWIFI.data.bytes[2], incWIFI.data.bytes[3], incWIFI.data.bytes[4], incWIFI.data.bytes[5], incWIFI.data.bytes[6], incWIFI.data.bytes[7]);
					SERIAL_CAPTURE(buffer);
				}
				else
				{
					// packet failed restart
					state = START_BYTE;
				}
				break;
			}
		}
	}
	return true;
}

// Displays CAN traffic on Serial out
bool CANBus::SDOutCAN(uint8_t config)
{
	// Display CAN0
	if (config == 1 && Can0.get_rx_buff(incCAN0))
	{
		sprintf(buffer, "%08d   %04X   %d   %02X  %02X  %02X  %02X  %02X  %02X  %02X  %02X\r\n", millis(), incCAN0.id, incCAN0.length, incCAN0.data.bytes[0], incCAN0.data.bytes[1], incCAN0.data.bytes[2], incCAN0.data.bytes[3], incCAN0.data.bytes[4], incCAN0.data.bytes[5], incCAN0.data.bytes[6], incCAN0.data.bytes[7]);
		SD_CAPTURE(buffer);
	}
	// Display CAN1
	else if (config == 2 && Can1.get_rx_buff(incCAN1))
	{
		sprintf(buffer, "%08d   %04X   %d   %02X  %02X  %02X  %02X  %02X  %02X  %02X  %02X\r\n", millis(), incCAN1.id, incCAN1.length, incCAN1.data.bytes[0], incCAN1.data.bytes[1], incCAN1.data.bytes[2], incCAN1.data.bytes[3], incCAN1.data.bytes[4], incCAN1.data.bytes[5], incCAN1.data.bytes[6], incCAN1.data.bytes[7]);
		SD_CAPTURE(buffer);
	}
	// Display CAN0 & CAN1
	else if (config == 3)
	{
		if (Can0.get_rx_buff(incCAN0))
		{
			sprintf(buffer, "%08d   %04X   %d   %02X  %02X  %02X  %02X  %02X  %02X  %02X  %02X\r\n", millis(), incCAN0.id, incCAN0.length, incCAN0.data.bytes[0], incCAN0.data.bytes[1], incCAN0.data.bytes[2], incCAN0.data.bytes[3], incCAN0.data.bytes[4], incCAN0.data.bytes[5], incCAN0.data.bytes[6], incCAN0.data.bytes[7]);
			SD_CAPTURE(buffer);
		}
		if (Can1.get_rx_buff(incCAN1))
		{
			sprintf(buffer, "%08d   %04X   %d   %02X  %02X  %02X  %02X  %02X  %02X  %02X  %02X\r\n", millis(), incCAN1.id, incCAN1.length, incCAN1.data.bytes[0], incCAN1.data.bytes[1], incCAN1.data.bytes[2], incCAN1.data.bytes[3], incCAN1.data.bytes[4], incCAN1.data.bytes[5], incCAN1.data.bytes[6], incCAN1.data.bytes[7]);
			SD_CAPTURE(buffer);
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
			SD_CAPTURE(buffer);
			Can0.sendFrame(incCAN1);
		}
	}
	// Forward traffic between CAN0-CAN1 and display CAN0 & CAN1
	else if (config == 5)
	{
		if (Can0.get_rx_buff(incCAN0))
		{
			sprintf(buffer, "%08d   %04X   %d   %02X  %02X  %02X  %02X  %02X  %02X  %02X  %02X\r\n", millis(), incCAN0.id, incCAN0.length, incCAN0.data.bytes[0], incCAN0.data.bytes[1], incCAN0.data.bytes[2], incCAN0.data.bytes[3], incCAN0.data.bytes[4], incCAN0.data.bytes[5], incCAN0.data.bytes[6], incCAN0.data.bytes[7]);
			SD_CAPTURE(buffer);
			Can1.sendFrame(incCAN0);
		}
		if (Can1.get_rx_buff(incCAN1))
		{
			sprintf(buffer, "%08d   %04X   %d   %02X  %02X  %02X  %02X  %02X  %02X  %02X  %02X\r\n", millis(), incCAN1.id, incCAN1.length, incCAN1.data.bytes[0], incCAN1.data.bytes[1], incCAN1.data.bytes[2], incCAN1.data.bytes[3], incCAN1.data.bytes[4], incCAN1.data.bytes[5], incCAN1.data.bytes[6], incCAN1.data.bytes[7]);
			SD_CAPTURE(buffer);
			Can0.sendFrame(incCAN1);
		}
	}
	return true;
}
