/*
	message.h - Header for the NymphMQTT message class.
	
	Revision 0
	
	Features:
			- 
			
	Notes:
			- 
			
	2019/05/08 - Maya Posch
*/


#ifndef NMQTT_MESSAGE_H
#define NMQTT_MESSAGE_H


#include <string>
#include <cstdint>

#include <bytebauble.h>


// Fixed header: first byte, bits [7-4].
enum MqttPacketType {
	MQTT_CONNECT = 0x10,
	MQTT_CONNACK = 0x20,
	MQTT_PUBLISH = 0x30,
	MQTT_PUBACK = 0x40,
	MQTT_PUBREC = 0x50,
	MQTT_PUBREL = 0x60,
	MQTT_PUBCOMP = 0x70,
	MQTT_SUBSCRIBE = 0x80,
	MQTT_SUBACK = 0x90,
	MQTT_UNSUBSCRIBE = 0xA0,
	MQTT_UNSUBACK = 0xB0,
	MQTT_PINGREQ = 0xC0,
	MQTT_PINGRESP = 0xD0,
	MQTT_DISCONNECT = 0xE0,
	MQTT_AUTH = 0xF0
};


// Fixed header: first byte, bits [3-0]
// These are all required values ('reserved' in the MQTT 5.0 spec) without further explanation.
// The Publish command is special, in that it sets the Duplicate, QoS and Retain flags.
enum MqttPacketTypeFlags {
	MQTT_FLAGS_PUBREL = 0x02,
	MQTT_FLAGS_SUBSCRIBE = 0x02,
	MQTT_FLAGS_UNSUBSCRIBE = 0x02
};


enum MqttQoS {
	MQTT_QOS_AT_MOST_ONCE = 0x0,
	MQTT_QOS_AT_LEAST_ONCE = 0x2,
	MQTT_QOS_EXACTLY_ONCE = 0x4
};


enum MqttConnectFlags {
	MQTT_CONNECT_CLEAN_START = 0x02,
	MQTT_CONNECT_WILL = 0x04,
	MQTT_CONNECT_WILL_QOS_L1 = 0x08,
	MQTT_CONNECT_WILL_QOS_L2 = 0x10,
	MQTT_CONNECT_WILL_RETAIN = 0x20,
	MQTT_CONNECT_PASSWORD = 0x40,
	MQTT_CONNECT_USERNAME = 0x80
};


enum MqttReasonCodes {
	MQTT_CODE_SUCCESS = 0x0,
	MQTT_CODE_MALFORMED_PACKET = 0x81,
	MQTT_CODE_PROTOCOL_ERROR = 0x82,
	MQTT_CODE_RECEIVE_MAX_EXCEEDED = 0x93,
	MQTT_CODE_PACKAGE_TOO_LARGE = 0x95,
	MQTT_CODE_RETAIN_UNSUPPORTED = 0x9A,
	MQTT_CODE_QOS_UNSUPPORTED = 0x9B,
	MQTT_CODE_SHARED_SUB_UNSUPPORTED = 0x9E,
	MQTT_CODE_SUB_ID_UNSUPPORTED = 0xA1,
	MQTT_CODE_WILD_SUB_UNSUPPORTED = 0xA2
};


class NmqttMessage {
	MqttPacketType command;
	
	// For Publish message.
	bool duplicateMessage = false;
	MqttQoS QoS = MQTT_QOS_AT_MOST_ONCE;
	bool retainMessage = false;
	
	// Fixed header.
	uint32_t messageLength;
	
	// Variable header.
	std::string topic;
	bool sessionPresent;
	MqttReasonCodes reasonCode;
	
	// Connect message.
	std::string will;
	std::string clientId;
	
	// Status flags.
	bool empty = true;		// Is this an empty message?
	bool parseGood = false; // Did the last binary message get parsed successfully?
	
	// Payload.
	std::string payload;
	
	ByteBauble bytebauble;
	
public:
	NmqttMessage();
	NmqttMessage(MqttPacketType type);
	NmqttMessage(std::string msg);
	~NmqttMessage();
	
	bool createMessage(MqttPacketType type);
	int parseMessage(std::string msg);
	bool valid() { return parseGood; }
	
	// For Connect message.
	void setWill(std::string will) { this->will = will; }
	void setClientId(std::string id) { clientId = id; }
	
	// For Publish message.
	void setDuplicateMessage(bool dup) { duplicateMessage = dup; }
	void setQoS(MqttQoS q) { QoS = q; }
	void setRetain(bool retain) { retainMessage = retain; }
	
	void setTopic(std::string topic) { this->topic = topic; }
	void setPayload(std::string payload) { this->payload = payload; }
	
	std::string getTopic() { return topic; }
	std::string getPayload() { return payload; }
	std::string getWill() { return will; }
	MqttReasonCodes getReasonCode() { return reasonCode; }
	
	std::string serialize();
};


#endif
