/*
	nmqtt_server.cpp - Main file of the NMQTT Server application.
	
	Revision 0.
	
	2021/01/04, Maya Posch
*/


#include "../cpp/server.h"

#include <iostream>
#include <vector>
#include <csignal>
#include <string>

#include <Poco/Condition.h>
#include <Poco/Thread.h>


Poco::Condition gCon;
Poco::Mutex gMutex;


void signal_handler(int signal) {
	gCon.signal();
}


// --- LOG FUNCTION ---
void logFunction(int level, std::string logStr) {
	std::cout << level << " - " << logStr << std::endl;
}


int main() {
	// Initialise the server instance.
	std::cout << "Initialising server..." << std::endl;
	long timeout = 5000; // 5 seconds.
	NmqttServer::init(logFunction, NYMPH_LOG_LEVEL_TRACE, timeout);
	
	// Install signal handler to terminate the server.
	signal(SIGINT, signal_handler);
	
	// Start server on port 1883.
	// FIXME: make configurable. Enable encrypted connections (8883).
	NmqttServer::start(1883);
	
	// Loop until the SIGINT signal has been received.
	gMutex.lock();
	gCon.wait(gMutex);
	
	// Clean-up
	NmqttServer::shutdown();
	
	// Wait before exiting, giving threads time to exit.
	Poco::Thread::sleep(2000); // 2 seconds.
	
	return 0; 
}
