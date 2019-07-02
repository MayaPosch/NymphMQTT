/*
	client_listener_manager.h	- Declares the NymphMQTT Listener Manager class.
	
	Revision 0
	
	Notes:
			- 
			
	History:
	2019/06/24, Maya Posch	: Initial version.
	
	(c) Nyanko.ws
*/


#ifndef NMQTT_CLIENT_LISTENER_MANAGER_H
#define NMQTT_CLIENT_LISTENER_MANAGER_H


#include <vector>
#include <string>

#include <Poco/Mutex.h>

#include "client_listener.h"


class NmqttClientListenerManager {
	static std::map<int, NmqttClientListener*> listeners;
	static Poco::Mutex listenersMutex;
	static std::string loggerName;
	
public:
	static void stop();
	
	static bool addConnection(int handle);
	static bool removeConnection(int handle);
};


#endif
