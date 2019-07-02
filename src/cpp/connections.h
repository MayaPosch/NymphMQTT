/*
	connections.h - Header for the NymphMQTT Connections class.
	
	Revision 0
	
	Features:
			- Static class to enable the global management of connections.
			
	Notes:
			- 
			
	2019/05/08 - Maya Posch
*/


#ifndef NMQTT_CONNECTIONS_H
#define NMQTT_CONNECTIONS_H


#include <map>
#include <functional>

#include <Poco/Semaphore.h>
#include <Poco/Net/StreamSocket.h>

#include "client.h"


// TYPES
struct NymphSocket {
	Poco::Net::StreamSocket* socket;	// Pointer to the socket instance.
	Poco::Semaphore* semaphore;			// Signals when it's safe to delete the socket.
	std::function<void(int, std::string, std::string)> handler;		// Publish message handler.
	std::function<void(int, bool, MqttReasonCodes)> connackHandler; // CONNACK handler.
	std::function<void(int)> pingrespHandler;						// PINGRESP handler.
	void* data;						// User data.
	int handle;						// The Nymph internal socket handle.
};


class NmqttConnections {
	static std::map<int, NymphSocket> sockets;
	
public:
	static void addSocket(NymphSocket &ns);
	static NymphSocket* getSocket(int handle);
};


#endif
