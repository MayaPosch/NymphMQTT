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


// Parses the provided binary message, setting the appropriate internal variables and status flags
// for reading out with other API functions.
NmqttMessage::NmqttMessage(std::string msg) {
	parseMessage(msg);
}


// --- DECONSTRUCTOR ---
NmqttMessage::~NmqttMessage() {
	//
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
	
	// Debug
	std::cout << "Found command: " << std::hex << (int) msg[0] << std::endl;
	
	// Read out the message length from the next 1-4 bytes.
	// TODO: refactor into loop.
	int lenBytes = 1;
	std::bitset<32> tLen;
	int tLenIdxB1 = 0;
	int tLenIdxB2 = 7;
	int tLenIdxB3 = 14;
	int tLenIdxB4 = 21;
	std::bitset<8> b1((uint8_t) msg[1]);
	
	// debug
	std::cout << "Bit 1: " << b1.to_string() << std::endl;
	
	if (b1.test(7)) {
		// Another byte follows this one.
		
		// debug
		std::cout << "Found another byte (1)." << std::endl;
		
		if (msg.size() < 3) {
			// TODO: Return error.
			return -1;
		}
		
		lenBytes++;
		
		std::bitset<8> b2((uint8_t) msg[2]);
		if (b2.test(7)) {
			// Third byte follows this one.
		
			// debug
			std::cout << "Found another byte (2)." << std::endl;
		
			if (msg.size() < 4) {
				// TODO: return error.
				return -1;
			}
			
			lenBytes++;
			
			std::bitset<8> b3((uint8_t) msg[3]);
			if (b3.test(7)) {
				// One more byte follows.				
				
				// debug
				std::cout << "Found another byte (3)." << std::endl;
				
				if (msg.size() < 5) {
					// TODO: return error.
					return -1;
				}
				
				lenBytes++;
				
				std::bitset<8> b4((uint8_t) msg[4]);
				
				// Add this value to the total length.
				// TODO:
				for (int i = 0; i < 7; ++i) {
					tLen[tLenIdxB4++] = b4[i];
				}
			}
			
			// Add the third byte's value to the total length.
			// TODO:
			for (int i = 0; i < 7; ++i) {
				tLen[tLenIdxB3++] = b3[i];
			}
		}
		
		// Add the second byte's value to the total length.
		// TODO:
		for (int i = 0; i < 7; ++i) {
			tLen[tLenIdxB2++] = b2[i];
		}
	}
	
	// Sum up the total message length remaining.
	for (int i = 0; i < 7; ++i) {
		tLen[tLenIdxB1++] = b1[i];
	}
	
	// Debug
	std::cout << "tLen: " << tLen.to_string() << std::endl;
	
	messageLength = tLen.to_ulong();
	idx = 1 + lenBytes;
	
	// debug
	std::cout << "Message length: " << messageLength << std::endl;
	
	if (lenBytes + 1 + messageLength != msg.length()) {
		// Return error.
		std::cerr << "Message length was wrong: expected " << lenBytes + 1 + messageLength
					<< ", got: " << msg.length() << std::endl;
		return -1;
	}
	
	// Read the variable header (if present).
	if (msg[0] == 0x30) {
		// Expect just the topic length (two bytes) and the topic string.
		// UTF-8 strings in MQTT have a big-endian, two-byte length header.
		// FIXME: just ignoring the MSB for now. Add endianness stuff to handle this.
		idx++; // Skip MSB.
		uint16_t strlen = (uint8_t) msg[idx++];
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
