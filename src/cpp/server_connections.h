/*
	server_connections.h - Header for the NymphMQTT Server Connections class.
	
	Revision 0
	
	Features:
			- Static class to enable the global management of client connections.
			
	Notes:
			- 
			
	2021/01/04 - Maya Posch
*/


#ifndef NMQTT_CLIENT_CONNECTIONS_H
#define NMQTT_CLIENT_CONNECTIONS_H


#include <map>
#include <queue>
#include <functional>

#include <Poco/Semaphore.h>
#include <Poco/Net/StreamSocket.h>
#include <Poco/Net/SecureStreamSocket.h>

#include "client.h"


// TYPES
struct NmqttClientSocket {
	//bool secure;						// Are using an SSL/TLS connection or not?
	//Poco::Net::SecureStreamSocket* ssocket;	// Pointer to a secure socket instance.
	//Poco::Net::Context::Ptr context;	// The security context for TLS connections.
	Poco::Net::StreamSocket* socket;	// Pointer to a non-secure socket instance.
	//Poco::Semaphore* semaphore;			// Signals when it's safe to delete the socket.
	//std::function<void(int, std::string, std::string)> handler;		// Publish message handler.
	std::function<void(uint64_t)> connectHandler;	// CONNECT handler.
	std::function<void(uint64_t)> pingreqHandler;	// PINGREQ handler.
	//void* data;						// User data.
	//int handle;						// The Nymph internal socket handle.
	bool username;
	bool password;
	bool willRetain;
	uint8_t qos;
	bool willFlag;
	bool cleanSession;
};


class NmqttClientConnections {
	static std::map<uint64_t, NmqttClientSocket> sockets;
	static uint64_t lastHandle;
	static std::queue<uint64_t> freeHandles;
	static NmqttClientSocket coreCS;
	
public:
	static uint64_t addSocket(NmqttClientSocket &ns);
	static NmqttClientSocket* getSocket(uint64_t handle);
	static void removeSocket(uint64_t handle);
	static void setCoreParameters(NmqttClientSocket &ns);
};


#endif
