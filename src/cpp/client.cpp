/*
	client.cpp - Implementation for the NymphMQTT Client class.
	
	Revision 0
	
	Features:
			- 
			
	Notes:
			- 
			
	2019/05/08 - Maya Posch
*/


#include "client.h"
#include "message.h"


// --- CONSTRUCTOR ---
NmqttClient::NmqttClient() {
	//
}


// --- SET WILL ---
// Set the will message for a Connect message.
void setWill(std::string will) {
	this->will = will;
}


bool connect(std::string host, int port) {
	// Create a Connect message, send it to the indicated remote.
	NmqttMessage msg(MQTT_CONNECT);
	msg.setWill(will);
	
}


bool publish(std::string topic, std::string payload, MqttQoS qos = MQTT_QOS_AT_MOST_ONCE, bool retain = false) {
	//
	NmqttMessage msg(MQTT_PUBLISH);
	msg.setQoS(qos);
	msg.setRetain(bool retain);
	msg.setTopic(topic);
	msg.setPayload(payload);
	
	std::string binMsg = msg.serialize();
}
