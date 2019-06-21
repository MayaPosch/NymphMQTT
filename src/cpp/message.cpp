/*
	message.cpp - Implementation for the NymphMQTT message class.
	
	Revision 0
	
	Features:
			- 
			
	Notes:
			- An MQTT message always consists out of three sections: 
				- Fixed header
				- Variable header (some messages)
				- Payload
				
			- Here the payload is just a binary string, only defined by a length parameter.
			- The fixed header defines the message type and flags specific to the type, along with
				the length of the message.
			- The variable header 
			
	2019/05/08 - Maya Posch
*/


#include "message.h"

#include <bitset>

// debug
#include <iostream>


// --- CONSTRUCTOR ---
// Default. Create empty message.
NmqttMessage::NmqttMessage() {
	//
}


// Create new message with command.
NmqttMessage::NmqttMessage(MqttPacketType type) {
	createMessage(type);
}


// Parses the provided binary message, setting the appropriate internal variables and status flags
// for reading out with other API functions.
NmqttMessage::NmqttMessage(std::string msg) {
	parseMessage(msg);
}


// --- DECONSTRUCTOR ---
NmqttMessage::~NmqttMessage() {
	//
}


// --- CREATE MESSAGE ---
//
bool NmqttMessage::createMessage(MqttPacketType type) {
	command = type;
	
	return true;
}


// --- PARSE MESSAGE ---
int NmqttMessage::parseMessage(std::string msg) {
	// Set initial flags.
	parseGood = false;
	
	int idx = 0;
	
	// Start by reading the fixed header, determining which command we're dealing with and the
	// remaining message length.
	// The message should be at least two bytes long (fixed header).
	if (msg.size() < 2) {
		// TODO: Return error.
		return -1;
	}
	
	// Debug
	std::cout << "Message: " << std::hex << msg << std::endl;
	
	// Read out the first byte.
	std::bitset<8> b0((uint8_t) msg[0]);
	idx++;
	
	// Debug
	std::cout << "Found command: " << std::hex << (int) msg[0] << std::endl;
		
	// Get the message length decoded using ByteBauble's method.
	int pblen = bytebauble.readPackedInt((uint32_t) msg[1], messageLength);
	idx += pblen;
	
	// debug
	std::cout << "Packed integer length: " << pblen << " bytes.\n";
	std::cout << "Message length: " << messageLength << std::endl;
	
	if (pblen + 1 + messageLength != msg.length()) {
		// Return error.
		std::cerr << "Message length was wrong: expected " << pblen + 1 + messageLength
					<< ", got: " << msg.length() << std::endl;
		return -1;
	}
	
	// Read the variable header (if present).
	if (msg[0] == 0x30) {
		// Expect just the topic length (two bytes) and the topic string.
		// UTF-8 strings in MQTT have a big-endian, two-byte length header.
		uint16_t lenBE = *((uint16_t*) &msg[idx]);
		
		// Debug
		std::cout << "String length (BE): 0x" << std::hex << lenBE << std::endl;
		
		uint16_t strlen = bytebauble.toHost(lenBE, BB_BE);
		idx += 2;
		topic = msg.substr(idx, strlen);
		
		// Debug
		std::cout << "Strlen: " << strlen << ", topic: " << topic << std::endl;
		
		idx += strlen;
	
		// TODO: handle QoS 1+ here.
		
		// Expect no properties here (0x00).
		
		// Debug
		std::cout << "Index for properties: " << idx << std::endl;
		
		uint8_t properties = msg[idx++];
		if (properties != 0x00) {
			std::cerr << "Expected no properties. Got: " << (int) properties << std::endl;
		}
		
		// Read the payload. This is the remaining section of the message (if any).
		if (idx + 1 != msg.length()) {
			payload = msg.substr(idx);
		}
	}
	
	// Set new flags.
	parseGood = true;
	return 1;
}


// --- SERIALIZE ---
std::string NmqttMessage::serialize() {
	// First byte contains the command and any flags.
	std::bitset<8> b0 = command;
	
	
	// Second byte is a Variable Byte, indicating the length of the rest of the message.
	
	// Next is the optional header section.
	
	// Finally the payload section. This is present for the following message types:
	// CONNECT 		Required
	// PUBLISH 		Optional
	// SUBSCRIBE 	Required
	// SUBACK 		Required
	// UNSUBSCRIBE 	Required
	// UNSUBACK 	Required
	
	return std::string();
	
}
