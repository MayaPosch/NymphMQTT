/*
	message_publish_test.cpp - Publish message test for the NymphMQTT Message class.
	
	Revision 0.
	
	2019/05/09, Maya Posch
*/


#include "../cpp/message.h"

#include <string>
#include <iostream>


int main() {
	// 
	
	std::string topic = "a/hello";
	std::string payload = "Hello World!";
	
	NmqttMessage msg(MQTT_PUBLISH);
	msg.setQoS(MQTT_QOS_AT_MOST_ONCE);
	msg.setRetain(false);
	msg.setTopic(topic);
	msg.setPayload(payload);
	
	std::string binMsg = msg.serialize();
	
	std::cout << "Bin msg: " << std::hex << binMsg << std::endl;
	std::cout << "Bin msg length: " << std::hex << binMsg.length() << std::endl;
	
	NmqttMessage msg2;
	msg2.parseMessage(binMsg);
	
	// Validate the results.
	if (!msg2.valid()) {
		std::cerr << "Failed to parse message." << std::endl;
		return 1;
	}
	
	std::cout << "Successfully parsed message." << std::endl;
	
	std::cout << "Found topic: " << msg2.getTopic() << std::endl;
	std::cout << "Found payload: " << msg2.getPayload() << std::endl;
	
	return 0;
}
