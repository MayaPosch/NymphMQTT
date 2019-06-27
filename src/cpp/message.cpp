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

#include <bytebauble.h>

// debug
#include <iostream>


// --- CONSTRUCTOR ---
// Default. Create empty message.
NmqttMessage::NmqttMessage() {
	//
}


// Create new message with command.
// Sets the invalid state if setting the command failed.
NmqttMessage::NmqttMessage(MqttPacketType type) {
	if (!createMessage(type)) { parseGood = false; }
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
// Set the command type. Returns false if the command type is invalid, for example when the current
// protocol version does not support it.
bool NmqttMessage::createMessage(MqttPacketType type) {
	if (type == MQTT_AUTH && mqttVersion == MQTT_PROTOCOL_VERSION_4) { return false; }
	
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
	std::cout << "Message(0): " << std::hex << (uint32_t) msg[0] << std::endl;
	std::cout << "Message(1): " << std::hex << (uint32_t) msg[1] << std::endl;
	std::cout << "Length: " << msg.length() << std::endl;
	
	// Read out the first byte.
	command = (MqttPacketType) msg[0]; // TODO: validate range.
	idx++;
	
	// Debug
	std::cout << "Found command: " << std::hex << (int) msg[0] << std::endl;
		
	// Get the message length decoded using ByteBauble's method.
	uint32_t pInt = (uint32_t) msg[1];
	int pblen = bytebauble.readPackedInt(pInt, messageLength);
	idx += pblen;
	
	// debug
	std::cout << "Variable integer: " << std::hex << pInt << std::endl;
	std::cout << "Packed integer length: " << pblen << " bytes." << std::endl;
	std::cout << "Message length: " << messageLength << std::endl;
	
	if (pblen + 1 + messageLength != msg.length()) {
		// Return error.
		std::cerr << "Message length was wrong: expected " << pblen + 1 + messageLength
					<< ", got: " << msg.length() << std::endl;
		return -1;
	}
	
	// Read the variable header (if present).
	switch ((uint8_t) msg[0]) {
		case MQTT_CONNECT: {
			// TODO: implement for server.
		}
		
		break;
		case MQTT_CONNACK: {
			// Basic parse: just get the variable header up till the reason code.
			sessionPresent = msg[idx++];
			reasonCode = (MqttReasonCodes) msg[idx++];
		}
		
		break;
		case MQTT_PUBLISH: {
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
		
		break;
		case MQTT_PUBACK: {
			//
		}
		
		break;
		case MQTT_PUBREC: {
			//
		}
		
		break;
		case MQTT_PUBREL: {
			//
		}
		
		break;
		case MQTT_PUBCOMP: {
			//
		}
		
		break;
		case MQTT_SUBSCRIBE: {
			//
		}
		
		break;
		case MQTT_SUBACK: {
			//
		}
		
		break;
		case MQTT_UNSUBSCRIBE: {
			//
		}
		
		break;
		case MQTT_UNSUBACK: {
			//
		}
		
		break;
		case MQTT_PINGREQ: {
			//
		}
		
		break;
		case MQTT_PINGRESP: {
			//
		}
		
		break;
		case MQTT_DISCONNECT: {
			//
		}
		
		break;
		case MQTT_AUTH: {
			//
		}
		
		break;
	};
	
	// Set new flags.
	parseGood = true;
	return 1;
}


// --- PARSE HEADER ---
// Returns a code to indicate whether the provided buffer contains a full MQTT fixed header section.
// Provides the parsed remaining message length and current index into the buffer where the section
// after the fixed header starts.
//
// Return codes:
// * (-1)	corrupted data.
// * (0) 	more bytes needed.
// * (1) 	successful parse.
int NmqttMessage::parseHeader(char* buff, int len, uint32_t &msglen, int& idx) {
	// Assume the first byte is fine. This will be handled later during the full message parsing.
	// Second byte starts the variable byte integer. Check up to four bytes whether they have 
	// bit 7 set. First three bytes means another byte follows, fourth byte means corrupted data.
	if (len < 2) { return -1; }
	
	idx = 1;
	
	if ((buff[idx++] >> 7) & 1UL) {
		if (len < 3) { return 0; }
	
		if ((buff[idx++] >> 7) & 1UL) {
			if (len < 4) { return 0; }
	
			if ((buff[idx++] >> 7) & 1UL) {
				if (len < 5) { return 0; }
		
				if ((buff[idx++] >> 7) & 1UL) {
					return -1; // Special bit on final byte should never be set.
				}
			}
		}

	}
	
	// Use ByteBauble to decode this variable byte integer.
	uint32_t pInt = *(uint32_t*) &buff[1]; // FIXME: limit?
	idx = ByteBauble::readPackedInt(pInt, msglen);
	idx++;
	
	return 1;
}


// --- SERIALIZE ---
std::string NmqttMessage::serialize() {
	// First byte contains the command and any flags.
	// Second byte is a Variable Byte, indicating the length of the rest of the message.
	// We'll fill this one in later.
	
	// Next is the optional (variable) header section.
	// This is present for these types:
	// 
	// PUBLISH		Required
	
	// Finally the payload section. This is present for the following message types:
	// CONNECT 		Required
	// PUBLISH 		Optional
	// SUBSCRIBE 	Required
	// SUBACK 		Required
	// UNSUBSCRIBE 	Required
	// UNSUBACK 	Required
	uint8_t b0 = command;
	std::string varHeader;
	bytebauble.setGlobalEndianness(BB_BE);
	switch (command) {
		case MQTT_CONNECT: {
			// Fixed header has no flags.
			
			// The Variable Header for the CONNECT Packet contains the following fields in this 
			// order: 
			// * Protocol Name, 
			// * Protocol Level, 
			// * Connect Flags, 
			// * Keep Alive, and
			// * Properties.
			bytebauble.setGlobalEndianness(BB_BE);
			uint16_t protNameLenHost = 0x0004;
			uint16_t protNameLenBE = bytebauble.toGlobal(protNameLenHost, bytebauble.getHostEndian());
			varHeader.append((char*) &protNameLenBE, 2);
			varHeader += "MQTT"; // The fixed protocol name.
			
			uint8_t protVersion = 4;	// Protocol version is 5.
			varHeader.append((char*) &protVersion, 1);
			
			uint8_t connectFlags = 0;
			connectFlags += (uint8_t) MQTT_CONNECT_CLEAN_START;
			//connectFlags += (uint8_t) MQTT_CONNECT_WILL;
			//connectFlags += (uint8_t) MQTT_CONNECT_WILL_QOS_L1;
			//connectFlags += (uint8_t) MQTT_CONNECT_WILL_RETAIN;			
			// TODO: username & password options.
			varHeader.append((char*) &connectFlags, 1);
			
			uint16_t keepAliveHost = 60; // In seconds.
			uint16_t keepAliveBE = bytebauble.toGlobal(keepAliveHost, bytebauble.getHostEndian());
			varHeader.append((char*) &keepAliveBE, 2);
			
			if (mqttVersion == MQTT_PROTOCOL_VERSION_5) {
				uint8_t propertiesLen = 0x05;
				uint8_t sessionExpIntervalId = 0x11;
				uint32_t sessionExpInterval = 0x0A;
				varHeader.append((char*) &propertiesLen, 1);
				varHeader.append((char*) &sessionExpIntervalId, 1);
				varHeader.append((char*) &sessionExpInterval, 4);
			}
			
			// The payload section depends on previously set flags.
			// These fields, if present, MUST appear in the order Client Identifier,
			// Will Properties, Will Topic, Will Payload, User Name, Password.
			
			// UTF8-encoded string with 16-bit uint BE header indicating string length.
			uint16_t clientIdLenHost = clientId.length();
			uint16_t clientIdLenBE = bytebauble.toGlobal(clientIdLenHost, bytebauble.getHostEndian());
			//std::cout << "ClientID: " << clientId << ", size: " << clientIdLenHost << ", BE: " <<
			//			std::hex << clientIdLenBE << std::endl;
			payload.append((char*) &clientIdLenBE, 2);
			payload += clientId;
			
			// TODO: Will properties, will topic, will payload.
			
			// TODO: username, password.
		}
		
		break;
		case MQTT_CONNACK: {
			// Fixed header has no flags.
			
			// The Variable Header of the CONNACK Packet contains the following fields in the order: 
			// * Connect Acknowledge Flags
			// * Connect Reason Code
			// * Properties.
			
			// Connect acknowledge flags. 1 byte. Bits 1-7 are reserved and set to 0.
			// Bit 0 is the session present flag. It's set to 0 if no existing session exists, or
			// the clean session flag was set in the Connect message.
			uint8_t connAckFlags = 0x0; // TODO: allow setting of this property.
			varHeader.append((char*) &connAckFlags, 1);
			
			// Connect reason code.
			// Single byte indicating the result of the connection attempt.
			uint8_t connRes = 0; // TODO: make settable.
			varHeader.append((char*) &connRes, 1);
			
			// Properties.
			// TODO: implement.
		}
		
		break;
		case MQTT_PUBLISH: {
			// Add flags as required.
			if (duplicateMessage) { b0 += 8; }
			if (QoS == MQTT_QOS_AT_LEAST_ONCE) { b0 += 2; }
			if (QoS == MQTT_QOS_EXACTLY_ONCE) { b0 += 4; }
			if (retainMessage) { b0 += 1; }
			
			// Variable header.
			// Get the length of the topic, convert it to big endian format.
			uint16_t topLenHost = topic.length();
			uint16_t topLenBE = bytebauble.toGlobal(topLenHost, bytebauble.getHostEndian());
			
			varHeader.append((char*) &topLenBE, 2);
			varHeader += topic;
			
			// Add packet identifier if QoS > 0.
			// TODO:
			
			// Set properties. 
			// TODO: implement. Set to 0 for now.
			uint8_t properties = 0x00;
			varHeader.append((char*) &properties, 1);
		}
		
		break;
		case MQTT_PUBACK: {
			//
		}
		
		break;
		case MQTT_PUBREC: {
			//
		}
		
		break;
		case MQTT_PUBREL: {
			//
		}
		
		break;
		case MQTT_PUBCOMP: {
			//
		}
		
		break;
		case MQTT_SUBSCRIBE: {
			// Fixed header has one required value: 0x2.
			b0 += 0x2;
			
			// Variable header. 
			uint16_t packetIdHost = 10;
			uint16_t packetIdBE = bytebauble.toGlobal(packetIdHost, bytebauble.getHostEndian());
			varHeader.append((char*) &packetIdBE, 2);
			
			// FIXME: MQTT 5.0
			/* uint8_t propLength = 0;
			varHeader.append((char*) &propLength, 1); */
			
			// Payload.
			uint16_t topicLenHost = topic.length();
			uint16_t topicLenBE = bytebauble.toGlobal(topicLenHost, bytebauble.getHostEndian());
			payload.append((char*) &topicLenBE, 2);
			payload += topic;
			
			// Subscribe flags.
			// TODO: implement settability.
			uint8_t subFlags = 0;
			payload.append((char*) &subFlags, 1);
		}
		
		break;
		case MQTT_SUBACK: {
			//
		}
		
		break;
		case MQTT_UNSUBSCRIBE: {
			//
		}
		
		break;
		case MQTT_UNSUBACK: {
			//
		}
		
		break;
		case MQTT_PINGREQ: {
			//
		}
		
		break;
		case MQTT_PINGRESP: {
			//
		}
		
		break;
		case MQTT_DISCONNECT: {
			//
		}
		
		break;
		case MQTT_AUTH: {
			//
		}
		
		break;
	};
		
	// Calculate message length after the fixed header. Encode as packed integer.
	uint32_t msgLen = varHeader.length() + payload.length();
	uint32_t msgLenPacked;
	uint32_t lenBytes = bytebauble.writePackedInt(msgLen, msgLenPacked);
	
	// Debug
	std::cout << "Message length: 0x" << std::hex << msgLen << std::endl;
	std::cout << "Message length (packed): 0x" << std::hex << msgLenPacked << std::endl;
	std::cout << "Message length bytes: 0x" << std::hex << lenBytes << std::endl;
	
	std::string output; // TODO: preallocate size.
	output.append((char*) &b0, 1);
	output.append(((char*) &msgLenPacked), lenBytes); // FIXME: validate
	output += varHeader;
	output += payload;
	
	return output;
	
}
