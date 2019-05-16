/*
	message.h - Header for the NymphMQTT message class.
	
	Revision 0
	
	Features:
			- 
			
	Notes:
			- 
			
	2019/05/08 - Maya Posch
*/


#include <string>
#include <cstdint>


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
	MQTT_QOS_AT_LEAST_ONCE = 0x1,
	MQTT_QOS_EXACTLY_ONCE = 0x2
};


class NmqttMessage {
	// For Publish message.
	bool duplicateMessage;
	MqttQoS QoS;
	bool retainMessage;
	
	// Fixed header.
	uint32_t messageLength;
	
	// Variable header.
	std::string topic;
	
	// Status flags.
	bool empty = true;		// Is this an empty message?
	bool parseGood = false; // Did the last binary message get parsed successfully?
	
	// Payload.
	std::string payload;
	
public:
	NmqttMessage();
	NmqttMessage(std::string msg);
	~NmqttMessage();
	
	int parseMessage(std::string msg);
	bool valid() { return parseGood; }
	
	void setWill(std::string will);
	bool connect(std::string host, int port);
	bool publish(std::string topic, std::string payload, int qos = 0, bool retain = false);
	
	std::string getTopic() { return topic; }
	std::string getPayload() { return payload; }
	
	std::string serialize();
};
