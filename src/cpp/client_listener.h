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
#include "message.h"
#include "connections.h"

#include <map>
#include <string>


class NmqttClientListener : public Poco::Runnable {
	std::string loggerName;
	bool listen;
	NymphSocket* nymphSocket;
	Poco::Net::StreamSocket* socket;
	bool init;
	Poco::Condition* readyCond;
	Poco::Mutex* readyMutex;
	
public:
	NmqttClientListener(int handle , Poco::Condition* cond, Poco::Mutex* mtx);
	~NmqttClientListener();
	void run();
	void stop();
};

#endif
