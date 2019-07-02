/*
	client_broker_test.cpp - Broker test for the NymphMQTT Client class.
	
	Revision 0.
	
	2019/05/09, Maya Posch
*/


#include "../cpp/client.h"

#include <string>
#include <iostream>

#include <sstream>

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


void messageHandler(int handle, std::string topic, std::string payload) {
	std::cout << "New message:" << std::endl;
	std::cout << "\tHandle: " << handle << std::endl;
	std::cout << "\tTopic: " << topic << std::endl;
	std::cout << "\tPayload: " << payload << std::endl;
}


int main() {
	// Do locale initialisation here to appease Valgrind (prevent data-race reporting).
	std::ostringstream dummy;
	dummy << 0;
	
	// Create client instance, connect to remote broker.
	NmqttClient client;
	client.init(logFunction, NYMPH_LOG_LEVEL_TRACE);
	client.setMessageHandler(messageHandler);
	
	Poco::Thread::sleep(500);
	
	std::cout << "TEST: Initialised client." << std::endl;
	
	
	std::string result;
	int handle;
	NmqttBrokerConnection conn;
	if (!client.connect("localhost", 1883, handle, 0, conn, result)) {
		std::cout << "Failed to connect to broker: " << result << std::endl;
		std::cout << "Got reason code: 0x" << (int) conn.responseCode << std::endl;
		client.shutdown();
		return 1;
	}
	
	Poco::Thread::sleep(500);
	
	std::cout << "TEST: Connected to broker." << std::endl;
	std::cout << "Got reason code: 0x" << (int) conn.responseCode << std::endl;
	
	// Subscribe to test topic.
	std::string topic = "a/hello";
	if (!client.subscribe(handle, topic, result)) {
		std::cout << "Failed to subscribe to topic:" << result << std::endl;
		client.shutdown();
		return 1;
	}
	
	Poco::Thread::sleep(500);
	
	std::cout << "TEST: Subscribed to topic." << std::endl;
	
	// Publish to test topic.
	std::string payload = "Hello World!";
	if (!client.publish(handle, topic, payload, result)) {
		std::cout << "Failed to publish to topic: " << result << std::endl;
		client.shutdown();
		return 1;
	}
	
	Poco::Thread::sleep(500);
	
	std::cout << "TEST: Published to topic." << std::endl;
	
	// Wait until the SIGINT signal has been received.
	gMutex.lock();
	gCon.wait(gMutex);
	
	Poco::Thread::sleep(500);
	
	std::cout << "TEST: Disconnecting from broker..." << std::endl;
	
	// Clean-up.
	if (!client.disconnect(handle, result)) {
		std::cerr << "Failed to disconnect from broker: " << result << std::endl;
		client.shutdown();
		return 1;
	}
	
	Poco::Thread::sleep(500);
	
	std::cout << "TEST: Shutting down client..." << std::endl;
	
	Poco::Thread::sleep(500);
	
	client.shutdown();
	
	return 0;
}