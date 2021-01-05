/*
	server.cpp - Definition of the NymphMQTT server class.
	
	Revision 0.
	
	Features:
			- Provides public API for NymphMQTT server.
			
	Notes:
			- 
			
	2021/01/03, Maya Posch
*/


#include "server.h"

#include "dispatcher.h"
#include "nymph_logger.h"
#include "session.h"
#include "server_connections.h"

#include <Poco/Net/NetException.h>
#include <Poco/NumberFormatter.h>


// Static initialisations.
long NmqttServer::timeout = 3000;
string NmqttServer::loggerName = "NmqttServer";
Poco::Net::ServerSocket NmqttServer::ss;
Poco::Net::TCPServer* NmqttServer::server;


// --- CONSTRUCTOR ---
NmqttServer::NmqttServer() {
	//
}


// --- DECONSTRUCTOR ---
NmqttServer::~NmqttServer() {
	//
}


// --- INIT ---
// Initialise the runtime.
bool NmqttServer::init(std::function<void(int, std::string)> logger, int level, long timeout) {
	NmqttServer::timeout = timeout;
	setLogger(logger, level);
	
	// Set the function pointers for the command handlers.
	NmqttClientSocket ns;
	//using namespace std::placeholders;
	ns.connectHandler = &NmqttServer::connectHandler; //std::bind(&NmqttServer::connectHandler, this, _1);
	ns.pingreqHandler = &NmqttServer::pingreqHandler; //std::bind(&NmqttServer::pingreqHandler, this, _1);
	NmqttClientConnections::setCoreParameters(ns);
	
	// Start the dispatcher runtime.
	// Get the number of concurrent threads supported by the system we are running on.
	int numThreads = std::thread::hardware_concurrency();
	
	// Initialise the Dispatcher with the maximum number of threads as worker count.
	Dispatcher::init(numThreads);
	
	return true;
}


// --- SET LOGGER ---
// Sets the logger function to be used by the Nymph Logger class, along with the
// desired maximum log level:
// NYMPH_LOG_LEVEL_FATAL = 1,
// NYMPH_LOG_LEVEL_CRITICAL,
// NYMPH_LOG_LEVEL_ERROR,
// NYMPH_LOG_LEVEL_WARNING,
// NYMPH_LOG_LEVEL_NOTICE,
// NYMPH_LOG_LEVEL_INFO,
// NYMPH_LOG_LEVEL_DEBUG,
// NYMPH_LOG_LEVEL_TRACE
void NmqttServer::setLogger(std::function<void(int, std::string)> logger, int level) {
	NymphLogger::setLoggerFunction(logger);
	NymphLogger::setLogLevel((Poco::Message::Priority) level);
}


// --- START ---
bool NmqttServer::start(int port) {
	try {
		// Create a server socket that listens on all interfaces, IPv4 and IPv6.
		// Assign it to the new TCPServer.
		ss.bind6(port, true, false); // Port, SO_REUSEADDR, IPv6-only.
		ss.listen();
		server = new Poco::Net::TCPServer(
					new Poco::Net::TCPServerConnectionFactoryImpl<NmqttSession>(), ss);
		server->start();
	}
	catch (Poco::Net::NetException& e) {
		NYMPH_LOG_ERROR("Error starting TCP server: " + e.message());
		return false;
	}
	
	//running = true;
	return true;
}


// --- SEND MESSAGE ---
// Private method for sending data to a remote broker.
bool NmqttServer::sendMessage(uint64_t handle, std::string binMsg) {
	NmqttClientSocket* clientSocket = NmqttClientConnections::getSocket(handle);
	
	try {
		int ret = clientSocket->socket->sendBytes(((const void*) binMsg.c_str()), binMsg.length());
		if (ret != binMsg.length()) {
			// Handle error.
			NYMPH_LOG_ERROR("Failed to send message. Not all bytes sent.");
			return false;
		}
		
		NYMPH_LOG_DEBUG("Sent " + Poco::NumberFormatter::format(ret) + " bytes.");
	}
	catch (Poco::Exception &e) {
		NYMPH_LOG_ERROR("Failed to send message: " + e.message());
		return false;
	}
	
	return true;
}


// --- CONNECT HANDLER ---
// Process connection. Return CONNACK response.
void NmqttServer::connectHandler(uint64_t handle) {
	NmqttMessage msg(MQTT_CONNACK);
	sendMessage(handle, msg.serialize());
}


// --- PINGREQ HANDLER ---
// Reply to ping response from a client.
void NmqttServer::pingreqHandler(uint64_t handle) {
	NmqttMessage msg(MQTT_PINGRESP);
	sendMessage(handle, msg.serialize());
}


// --- SHUTDOWN ---
// Shutdown the runtime. Close any open connections and clean up resources.
bool NmqttServer::shutdown() {
	server->stop();
	
	return true;
}
