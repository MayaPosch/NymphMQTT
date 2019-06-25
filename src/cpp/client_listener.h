/*
	client_listener.h - Header for the NymphMQTT Client socket listening thread class.
	
	Revision 0
	
	Features:
			- Socket listener for MQTT connections.
			
	Notes:
			- 
			
	2019/05/08 - Maya Posch
*/


#ifndef NMQTT_CLIENT_LISTENER_H
#define NMQTT_CLIENT_LISTENER_H


#include <Poco/Runnable.h>
#include <Poco/Net/StreamSocket.h>
#include <Poco/Semaphore.h>
#include <Poco/Condition.h>

#include "client.h"

#include <map>
#include <string>


// TYPES
struct NymphSocket {
	Poco::Net::StreamSocket* socket;	// Pointer to the socket instance.
	Poco::Semaphore* semaphore;			// Signals when it's safe to delete the socket.
	NymphMessageHandler handler;		// Publish message handler.
	void* data;						// User data.
	int handle;						// The Nymph internal socket handle.
};


class NmqttClientListener : public Poco::Runnable {
	std::string loggerName;
	bool listen;
	NymphSocket nymphSocket;
	Poco::Net::StreamSocket* socket;
	bool init;
	Poco::Condition* readyCond;
	Poco::Mutex* readyMutex;
	
public:
	NmqttClientListener(NymphSocket socket, Poco::Condition* cond, Poco::Mutex* mtx);
	~NmqttClientListener();
	void run();
	void stop();
};

#endif
