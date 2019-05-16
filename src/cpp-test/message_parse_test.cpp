/*
	message_parse_test.cpp - Parse test for the NymphMQTT Message class.
	
	Revision 0.
	
	2019/05/09, Maya Posch
*/


#include "../cpp/message.h"

#include <string>
#include <iostream>


int main() {
	// Create MQTT binary message, create NmqttMessage instance.
	// Test message is a Publish command, with the string payload 'Hello World!'.
	NmqttMessage msg;
	//uint8_t mqtt_arr[] = { };
	std::string mqtt_msg({0x30, 0x13, 0x00, 0x04, 'a', '/', 'h', 'i', 0x00});
	mqtt_msg.append("Hello World!");
	
	// Parse the binary message with the instance.
	msg.parseMessage(mqtt_msg);
	
	// Validate the results.
	if (!msg.valid()) {
		std::cerr << "Failed to parse message." << std::endl;
		return 1;
	}
	
	std::cout << "Successfully parsed message." << std::endl;
	
	std::cout << "Found topic: " << msg.getTopic() << std::endl;
	std::cout << "Found payload: " << msg.getPayload() << std::endl;
	
	return 0;
}
