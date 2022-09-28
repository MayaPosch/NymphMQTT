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
#include "nymph_logger.h"

#include <bytebauble.h>

#include <iostream>

// debug
//#define DEBUG 1
#ifdef DEBUG
#endif


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


// --- SET CREDENTIALS ---
void NmqttMessage::setCredentials(std::string &user, std::string &pass) {
	username = user;
	password = pass;
	usernameFlag = true;
	passwordFlag = true;
}


// --- SET WILL ---
void NmqttMessage::setWill(std::string topic, std::string will, uint8_t qos, bool retain) {
	this->will = will;
	this->willTopic = topic;
	this->willRetainFlag = retain;
	this->willQoS = qos;
	this->willFlag = true;
	if (qos == 1) { 
		willQoS1 = true;
	}
	else if (qos == 2) {
		willQoS2 = true;
	}
}


// --- CREATE MESSAGE ---
// Set the command type. Returns false if the command type is invalid, for example when the current
// protocol version does not support it.
bool NmqttMessage::createMessage(MqttPacketType type) {
	if (type == MQTT_AUTH && mqttVersion == MQTT_PROTOCOL_VERSION_4) { return false; }
	
	if (type == MQTT_CONNECT) {
		// Set default connect options.
		cleanSessionFlag = true;
		willFlag = false;
		willRetainFlag = false;
		usernameFlag = false;
		passwordFlag = false;
		willQoS1 = false;
		willQoS2 = false;
	}
	
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
#ifdef DEBUG
	std::cout << "Message(0): " << std::hex << (uint32_t) msg[0] << std::endl;
	std::cout << "Message(1): " << std::hex << (uint32_t) msg[1] << std::endl;
	std::cout << "Length: " << msg.length() << std::endl;
#endif
	
	// Read out the first byte. Mask bits 0-3 as they're not used here.
	// Also read out the DUP, QoS and Retain flags.
	uint8_t byte0 = static_cast<uint8_t>(msg[0]);
	command = (MqttPacketType) (byte0 & 0xF0); // TODO: validate range.
	duplicateMessage = (byte0 >> 3) & 1U;
	uint8_t qosCnt = 0;
	if ((byte0 >> 2) & 1U) { QoS = MQTT_QOS_AT_LEAST_ONCE; qosCnt++; }
	if ((byte0 >> 1) & 1U) { QoS = MQTT_QOS_EXACTLY_ONCE; qosCnt++; }
	if (qosCnt > 1) { QoS = MQTT_QOS_AT_MOST_ONCE; }
	retainMessage = byte0 & 1U;
	idx++;
	
	// Debug
#ifdef DEBUG
	std::cout << "Found command: 0x" << std::hex << command << std::endl;
#endif
		
	// Get the message length decoded using ByteBauble's method.
	uint32_t pInt = (uint32_t) msg[1];
	int pblen = bytebauble.readPackedInt(pInt, messageLength);
	idx += pblen;
	
	// debug
#ifdef DEBUG
	std::cout << "Variable integer: " << std::hex << pInt << std::endl;
	std::cout << "Packed integer length: " << pblen << " bytes." << std::endl;
	std::cout << "Message length: " << messageLength << std::endl;
#endif
	
	if (pblen + 1 + messageLength != msg.length()) {
		// Return error.
		std::cerr << "Message length was wrong: expected " << pblen + 1 + messageLength
					<< ", got: " << msg.length() << std::endl;
		return -1;
	}
	
	// Read the variable header (if present).
	switch (command) {
		case MQTT_CONNECT: {
			// Server.
			// Decode variable header.
			// First field: protocol name '0x00 0x04 M Q T T'.
			std::string protName = msg.substr(idx, 6);
			std::string protMatch = { 0x00, 0x04, 'M', 'Q', 'T', 'T' };
			if (protName != protMatch) {
				std::cerr << "CONNECT protocol name incorrect, got: " << protName << std::endl;
				return -1;
			}
			
			idx += 6;
			
			// Protocol level: one byte.
			uint8_t protver = (uint8_t) msg[idx++];
			if (protver == 4) { mqttVersion = MQTT_PROTOCOL_VERSION_4; }
			else if (protver == 5) { mqttVersion == MQTT_PROTOCOL_VERSION_5; }
			else {
				std::cerr << "Invalid MQTT version: " << (uint16_t) protver << std::endl;
				// FIXME: Server must return CONNACK with code 0x01 in this case.
				return -1;
			}
			
			// Connect flags: one byte.
			uint8_t connflags = (uint8_t) msg[idx++];
			usernameFlag = (connflags >> 7) & 1U;
			passwordFlag = (connflags >> 6) & 1U;
			willRetainFlag = (connflags >> 5) & 1U;
			willQoS = (((connflags >> 4) & 1U) << 1) + (connflags >> 3) & 1U;
			willFlag = (connflags >> 2) & 1U;
			cleanSessionFlag = (connflags >> 1) & 1U;
			if (connflags & 1U) {
				std::cerr << "Reserved flag in connect flags set. Aborting parse." << std::endl;
				return -1;
			}
			
			// Keep alive value: two bytes.
			keepAlive = (uint16_t) msg[idx];
			idx += 2;
			
			// Payload section.
			// Client ID. UTF-8 string, preceded by two bytes (MSB, LSB) with the length.
			// TODO: convert BE length int to host int.
			uint16_t len = (uint16_t) msg[idx];
			idx += 2;
			clientId = msg.substr(idx, len);
			idx += len;
			
			if (willFlag) {
				uint16_t len = (uint16_t) msg[idx];
				idx += 2;
				willTopic = msg.substr(idx, len);
				idx += len;
			
				len = (uint16_t) msg[idx];
				idx += 2;
				will = msg.substr(idx, len);
				idx += len;
			}
			
			if (usernameFlag) {
				uint16_t len = (uint16_t) msg[idx];
				idx += 2;
				username = msg.substr(idx, len);
				idx += len;
			}
			
			if (passwordFlag) {
				uint16_t len = (uint16_t) msg[idx];
				idx += 2;
				password = msg.substr(idx, len);
				idx += len;
			}
		}
		
		break;
		case MQTT_CONNACK: {
			NYMPH_LOG_INFORMATION("Received CONNACK message.");
			
			// Basic parse: just get the variable header up till the reason code.
			// For MQTT 5 we also need to read out the properties that follow after the reason code.
			sessionPresent = msg[idx++];
			reasonCode = (MqttReasonCodes) msg[idx++];
		}
		
		break;
		case MQTT_PUBLISH: {
			NYMPH_LOG_INFORMATION("Received PUBLISH message.");
			
			// Expect just the topic length (two bytes) and the topic string.
			// UTF-8 strings in MQTT have a big-endian, two-byte length header.
			uint16_t lenBE = *((uint16_t*) &msg[idx]);
			
			// Debug
#ifdef DEBUG
			std::cout << "String length (BE): 0x" << std::hex << lenBE << std::endl;
#endif
			
			uint16_t strlen = bytebauble.toHost(lenBE, BB_BE);
			idx += 2;
			topic = msg.substr(idx, strlen);
			
			// Debug
#ifdef DEBUG
			std::cout << "Strlen: " << strlen << ", topic: " << topic << std::endl;
#endif
			
			idx += strlen;
		
			// Handle QoS 1+ here.
			// Parse out the two bytes containing the packet identifier. This is in BE format
			// (MSB/LSB).
			if (qosCnt > 0) {
				uint16_t pIDBE = *((uint16_t*) &msg[idx]);
				packetID = bytebauble.toHost(pIDBE, BB_BE);
				idx += 2;
			}
			
			if (mqttVersion == MQTT_PROTOCOL_VERSION_5) {
				// MQTT 5: Expect no properties here (0x00).
				
				// Debug
#ifdef DEBUG
				std::cout << "Index for properties: " << idx << std::endl;
#endif
				
				uint8_t properties = msg[idx++];
				if (properties != 0x00) {
					std::cerr << "Expected no properties. Got: " << (int) properties << std::endl;
				}
			}
			
			// Read the payload. This is the remaining section of the message (if any).
			if (idx + 1 != msg.length()) {
				payload = msg.substr(idx);
			}
		}
		
		break;
		case MQTT_PUBACK: {
			NYMPH_LOG_INFORMATION("Received PUBACK message.");
			
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
			// Server.
			// TODO: implement.
			// 
		}
		
		break;
		case MQTT_SUBACK: {
			NYMPH_LOG_INFORMATION("Received SUBACK message.");
			
			//
		}
		
		break;
		case MQTT_UNSUBSCRIBE: {
			// Server.
			// TODO: implement.
			//
		}
		
		break;
		case MQTT_UNSUBACK: {
			// Client.
			//
			NYMPH_LOG_INFORMATION("Received UNSUBACK message.");
		}
		
		break;
		case MQTT_PINGREQ: {
			// Server.
			// No variable header or payload.
		}
		
		break;
		case MQTT_PINGRESP: {
			NYMPH_LOG_INFORMATION("Received PINGRESP message.");
			
			// A Ping response has no variable header and no payload.
		}
		
		break;
		case MQTT_DISCONNECT: {
			NYMPH_LOG_INFORMATION("Received DISCONNECT message.");
			
			// A disconnect message has no variable header and no payload.
		}
		
		break;
		case MQTT_AUTH: {
			// MQTT 5.
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
			// * Properties. (MQTT 5)
			bytebauble.setGlobalEndianness(BB_BE);
			uint16_t protNameLenHost = 0x0004;
			uint16_t protNameLenBE = bytebauble.toGlobal(protNameLenHost, bytebauble.getHostEndian());
			varHeader.append((char*) &protNameLenBE, 2);
			varHeader += "MQTT"; // The fixed protocol name.
			
			uint8_t protVersion;
			if (mqttVersion == MQTT_PROTOCOL_VERSION_5) {
				protVersion = 5;
			}
			else {
				protVersion = 4;	// Protocol version default is 4 (3.1.1).
			}
			
			varHeader.append((char*) &protVersion, 1);
			
			uint8_t connectFlags = 0;
			if (cleanSessionFlag) { connectFlags += (uint8_t) MQTT_CONNECT_CLEAN_START; }
			if (willFlag) { connectFlags += (uint8_t) MQTT_CONNECT_WILL; }
			if (willQoS1) { connectFlags += (uint8_t) MQTT_CONNECT_WILL_QOS_L1; }
			if (willQoS2) { connectFlags += (uint8_t) MQTT_CONNECT_WILL_QOS_L2; }
			if (willRetainFlag) { connectFlags += (uint8_t) MQTT_CONNECT_WILL_RETAIN; }
			if (passwordFlag) { connectFlags += (uint8_t) MQTT_CONNECT_PASSWORD; }
			if (usernameFlag) { connectFlags += (uint8_t) MQTT_CONNECT_USERNAME; }
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
			payload.append((char*) &clientIdLenBE, 2);
			payload += clientId;
			
			// Will properties (MQTT 5), will topic, will payload.
			if (willFlag) {
				uint16_t willTopicLenBE = bytebauble.toGlobal(willTopic.length(), 
																bytebauble.getHostEndian());
				payload.append((char*) &willTopicLenBE, 2);
				payload += willTopic;
				
				uint16_t willLenBE = bytebauble.toGlobal(will.length(), bytebauble.getHostEndian());
				payload.append((char*) &willLenBE, 2);
				payload += will;
			}
			
			// Username, password.
			if (usernameFlag) {
				uint16_t usernameLenBE = bytebauble.toGlobal(willTopic.length(), 
																bytebauble.getHostEndian());
				payload.append((char*) &usernameLenBE, 2);
				payload += username;
			}
				
			if (passwordFlag) {
				uint16_t passwordLenBE = bytebauble.toGlobal(password.length(), bytebauble.getHostEndian());
				payload.append((char*) &passwordLenBE, 2);
				payload += password;
			}
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
			// TODO: keep track of unused IDs.
			if (QoS != MQTT_QOS_AT_MOST_ONCE) {
				uint16_t pIDBE = bytebauble.toGlobal(packetID, bytebauble.getHostEndian());
				varHeader.append((char*) &pIDBE, 2);
			}
			
			// Set properties. 
			// TODO: implement. Set to 0 for now.
			if (mqttVersion == MQTT_PROTOCOL_VERSION_5) {
				uint8_t properties = 0x00;
				varHeader.append((char*) &properties, 1);
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
			// Fixed header has one required value: 0x2.
			b0 += 0x2;
			
			// Variable header. 
			uint16_t packetIdHost = 10;
			uint16_t packetIdBE = bytebauble.toGlobal(packetIdHost, bytebauble.getHostEndian());
			varHeader.append((char*) &packetIdBE, 2);
			
			if (mqttVersion == MQTT_PROTOCOL_VERSION_5) {
				uint8_t propLength = 0;
				varHeader.append((char*) &propLength, 1);
			}
			
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
			// Client.
			// Fixed header has one required value: 0x2.
			b0 += 0x2;
			
			// Variable header. 
			uint16_t packetIdHost = 10;
			uint16_t packetIdBE = bytebauble.toGlobal(packetIdHost, bytebauble.getHostEndian());
			varHeader.append((char*) &packetIdBE, 2);
			
			// Set packet ID.
			// TODO: implement packet ID handling.
			uint16_t pIDBE = bytebauble.toGlobal(packetID, bytebauble.getHostEndian());
			varHeader.append((char*) &pIDBE, 2);
			
			// Payload is the topic to unsubscribe from.
			uint16_t topicLenBE = bytebauble.toGlobal(topic.length(), bytebauble.getHostEndian());
			payload.append((char*) &topicLenBE, 2);
			payload += topic;
		}
		
		break;
		case MQTT_UNSUBACK: {
			//
		}
		
		break;
		case MQTT_PINGREQ: {
			// Server.
			// This command has no variable header and no payload.
			// Generate a PINGRESP ping response packet to send back to the client.
		}
		
		break;
		case MQTT_PINGRESP: {
			// A ping response has no variable header and no payload.
		}
		
		break;
		case MQTT_DISCONNECT: {
			// A disconnect message has no variable header and no payload.
		}
		
		break;
		case MQTT_AUTH: {
			// MQTT 5.
		}
		
		break;
	};
		
	// Calculate message length after the fixed header. Encode as packed integer.
	uint32_t msgLen = varHeader.length() + payload.length();
	uint32_t msgLenPacked;
	uint32_t lenBytes = bytebauble.writePackedInt(msgLen, msgLenPacked);
	
	// Debug
#ifdef DEBUG
	std::cout << "Message length: 0x" << std::hex << msgLen << std::endl;
	std::cout << "Message length (packed): 0x" << std::hex << msgLenPacked << std::endl;
	std::cout << "Message length bytes: 0x" << std::hex << lenBytes << std::endl;
#endif
	
	std::string output; // TODO: preallocate size.
	output.append((char*) &b0, 1);
	output.append(((char*) &msgLenPacked), lenBytes); // FIXME: validate
	output += varHeader;
	output += payload;
	
	return output;
	
}
