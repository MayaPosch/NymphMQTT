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

#include <Poco/Net/NetException.h>
#include <Poco/NumberFormatter.h>

using namespace Poco;

using namespace std;


// --- CONSTRUCTOR ---
NmqttClient::NmqttClient() {
	//
}


// --- INIT ---
// Initialise the runtime and sets the logger function to be used by the Nymph 
// Logger class, along with the desired maximum log level:
// NYMPH_LOG_LEVEL_FATAL = 1,
// NYMPH_LOG_LEVEL_CRITICAL,
// NYMPH_LOG_LEVEL_ERROR,
// NYMPH_LOG_LEVEL_WARNING,
// NYMPH_LOG_LEVEL_NOTICE,
// NYMPH_LOG_LEVEL_INFO,
// NYMPH_LOG_LEVEL_DEBUG,
// NYMPH_LOG_LEVEL_TRACE
bool NmqttClient::init(logFnc logger, int level, long timeout) {
	//NymphRemoteServer::timeout = timeout; // FIXME
	setLogger(logger, level);
	
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
void NmqttClient::setLogger(logFnc logger, int level) {
	NymphLogger::setLoggerFunction(logger);
	NymphLogger::setLogLevel((Poco::Message::Priority) level);
}


// --- SET MESSAGE HANDLER ---
// Set the callback function that will be called every time a message is received from the broker.
void NmqttClient::setMessageHandler(NymphMessageHandler handler) {
	messageHandler = handler;
}


// --- SHUTDOWN ---
// Shutdown the runtime. Close any open connections and clean up resources.
bool NmqttClient::shutdown() {
	socketsMutex.lock();
	map<int, Poco::Net::StreamSocket*>::iterator it;
	for (it = sockets.begin(); it != sockets.end(); ++it) {
		// Remove socket from listener.
		//NymphListener::removeConnection(it->first);
		
		// TODO: try/catch. 
		it->second->shutdown();
	}
	
	sockets.clear();
	socketsMutex.unlock();
	
	//NymphListener::stop();
	
	return true;
}

 
// --- CONNECT ---
// Create a new connection with the remote MQTT server and return a handle for
// the connection.
bool NmqttClient::connect(string host, int port, int &handle, void* data, 
															string &result) {
	Poco::Net::SocketAddress sa(host, port);
	return connect(sa, handle, data, result);
}


bool NmqttClient::connect(string url, int &handle, void* data, 
															string &result) {
	Poco::Net::SocketAddress sa(url);
	return connect(sa, handle, data, result);
}


bool NmqttClient::connect(Poco::Net::SocketAddress sa, int &handle, 
												void* data, string &result) {
	Poco::Net::StreamSocket* socket;
	try {
		socket = new Poco::Net::StreamSocket(sa);
	}
	catch (Poco::Net::ConnectionRefusedException &ex) {
		// Handle connection error.
		result = "Unable to connect: " + ex.displayText();
		return false;
	}
	catch (Poco::InvalidArgumentException &ex) {
		result = "Invalid argument: " + ex.displayText();
		return false;
	}
	catch (Poco::Net::InvalidSocketException &ex) {
		result = "Invalid socket exception: " + ex.displayText();
		return false;
	}
	catch (Poco::Net::NetException &ex) {
		result = "Net exception: " + ex.displayText();
		return false;
	}
	
	socketsMutex.lock();
	sockets.insert(pair<int, Poco::Net::StreamSocket*>(lastHandle, socket));
	NymphSocket ns;
	ns.socket = socket;
	ns.semaphore = new Semaphore(0, 1);
	socketSemaphores.insert(pair<int, Poco::Semaphore*>(lastHandle, ns.semaphore));
	ns.data = data;
	ns.handle = lastHandle;
	NymphListener::addConnection(lastHandle, ns);
	handle = lastHandle++;
	socketsMutex.unlock();
	
	NYMPH_LOG_DEBUG("Added new connection with handle: " + NumberFormatter::format(handle));
	
	// Send Connect message using the previously set data.
	NmqttMessage msg(MQTT_CONNECT);
	msg.setWill(will);
	msg.setClientId(clientId);
	
	return sendMessage(int handle, msg.serialize());
}


// --- DISCONNECT ---
bool NmqttClient::disconnect(int handle, string &result) {	
	
	// Create a Disconnect message, send it to the indicated remote.
	NmqttMessage msg(MQTT_DISCONNECT);
	msg.setWill(will);
	
	return sendMessage(int handle, msg.serialize());
	
	map<int, Poco::Net::StreamSocket*>::iterator it;
	map<int, Poco::Semaphore*>::iterator sit;
	socketsMutex.lock();
	it = sockets.find(handle);
	if (it == sockets.end()) { 
		result = "Provided handle " + NumberFormatter::format(handle) + " was not found.";
		socketsMutex.unlock();
		return false; 
	}
	
	sit = socketSemaphores.find(handle);
	if (sit == socketSemaphores.end()) {
		result = "No semaphore found for socket handle.";
		socketsMutex.unlock();
		return false;
	}
	
	// TODO: try/catch.
	// Shutdown socket. Set the semaphore once done to signal that the socket's 
	// listener thread that it's safe to delete the socket.
	it->second->shutdown();
	it->second->close();
	if (sit->second) { sit->second->set(); }
	
	// Remove socket from listener.
	NymphListener::removeConnection(it->first);
	
	// Remove socket references from both maps.
	sockets.erase(it);
	socketSemaphores.erase(sit);
	
	socketsMutex.unlock();
	
	NYMPH_LOG_DEBUG("Removed connection with handle: " + NumberFormatter::format(handle));
	
	return true;
}


// --- SET WILL ---
// Set the will message for a Connect message.
void NmqttClient::setWill(std::string will) {
	this->will = will;
}


/* bool NmqttClient::connect(std::string host, int port) {
	// Create a Connect message, send it to the indicated remote.
	NmqttMessage msg(MQTT_CONNECT);
	msg.setWill(will);
	
	std::string binMsg = msg.serialize();
	
	// Send request to remote server.
} */


// --- SEND MESSAGE ---
// Private method for sending data to a remote broker.
bool NmqttClient::sendMessage(int handle, std::string binMsg) {
	map<int, Poco::Net::StreamSocket*>::iterator it;
	socketsMutex.lock();
	it = sockets.find(handle);
	if (it == sockets.end()) { 
		//result = "Provided handle " + NumberFormatter::format(handle) + " was not found.";
		socketsMutex.unlock();
	}
	
	try {
		int ret = it->second->sendBytes(((const void*) binMsg.c_str()), binMsg.length());
		if (ret != binMsg.length()) {
			// Handle error.
			//result = "Failed to send message: ";		
			return false;
		}
		
		NYMPH_LOG_DEBUG("Sent " + NumberFormatter::format(ret) + " bytes.");
	}
	catch (Poco::Exception &e) {
		//result = "Failed to send message: " + e.message();
		return false;
	}
	
	return true;
}


// --- PUBLISH ---
bool NmqttClient::publish(int handle, std::string topic, std::string payload, std::string &result, 
							MqttQoS qos = MQTT_QOS_AT_MOST_ONCE, bool retain = false) {
	//
	NmqttMessage msg(MQTT_PUBLISH);
	msg.setQoS(qos);
	msg.setRetain(bool retain);
	msg.setTopic(topic);
	msg.setPayload(payload);
	
	return sendMessage(handle, msg.serialize());
}


// --- SUBSCRIBE ---
bool NmqttClient::subscribe(int handle, std::string topic, std::string result) {
	//
	NmqttMessage msg(MQTT_SUBSCRIBE);
	msg.setTopic(topic);
	
	return sendMessage(handle, msg.serialize());
}
