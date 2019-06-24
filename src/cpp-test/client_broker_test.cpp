/*
	client_broker_test.cpp - Broker test for the NymphMQTT Client class.
	
	Revision 0.
	
	2019/05/09, Maya Posch
*/


#include "../cpp/client.h"

#include <string>
#include <iostream>

#include <Poco/Condition.h>
#include <Poco/Thread.h>

using namespace Poco;


Condition gCon;
Mutex gMutex;


void signal_handler(int signal) {
	gCon.signal();
}


void logFunction(int level, std::string text) {
	std::cout << level << " - " << text << std::endl;
}


void messageHandler(std::string topic, std::string payload) {
	std::cout << "New message:" << std::endl;
	std::cout << "\tTopic: " << topic << std::endl;
	std::cout << "\tPayload: " << payload << std::endl;
}


int main() {
	// Create client instance, connect to remote broker.
	NmqttClient client;
	client.init(logFunction, NYMPH_LOG_LEVEL_TRACE);
	
	std::string result;
	int handle;
	if (!client.connect("localhost", 1883, handle, 0, result)) {
		std::cout << "Failed to connect to broker: " << result << std::endl;
		client.shutdown();
		return 1;
	}
	
	// Subscribe to test topic.
	std::string topic = "a/hello";
	if (!client.subscribe(handle, topic, result)) {
		std::cout << "Failed to subscribe to topic:" << result << std::endl;
		client.shutdown();
		return 1;
	}
	
	// Publish to test topic.
	std::string payload = "Hello World!";
	if (!client.publish(handle, topic, payload, result)) {
		std::cout << "Failed to publish to topic: " << result << std::endl;
		client.shutdown();
		return 1;
	}
	
	// Confirm reception test message on topic.
	
	// Wait until the SIGINT signal has been received.
	gMutex.lock();
	gCon.wait(gMutex);
	
	// Clean-up.
	if (!client.disconnect(handle, result)) {
		std::cerr << "Failed to disconnect from broker: " << result << std::endl;
		client.shutdown();
		return 1;
	}
	
	client.shutdown();
	
	return 0;
}