/*
	client.h - Header for the NymphMQTT Client class.
	
	Revision 0
	
	Features:
			- 
			
	Notes:
			- 
			
	2019/05/08 - Maya Posch
*/


#ifndef NMQTT_CLIENT_H
#define NMQTT_CLIENT_H


#include <string>
#include <map>

#include <Poco/Mutex.h>
#include <Poco/Semaphore.h>
#include <Poco/Net/SocketAddress.h>
#include <Poco/Net/StreamSocket.h>

#include "nymph_logger.h"
#include "message.h"


// Function pointer signatures.
typedef void (*NymphMessageHandler)(std::string topic, std::string payload);


class NmqttClient {
	std::map<int, Poco::Net::StreamSocket*> sockets;
	std::map<int, Poco::Semaphore*> socketSemaphores;
	Poco::Mutex socketsMutex;
	int lastHandle;
	long timeout;
	std::string loggerName;
	NymphMessageHandler messageHandler;
	
	std::string will;
	std::string clientId = "NymphMQTT-client";
	
	bool sendMessage(int handle, std::string binMsg);
	
public:
	NmqttClient();
	
	bool init(logFnc logger, int level = NYMPH_LOG_LEVEL_TRACE, long timeout = 3000);
	void setLogger(logFnc logger, int level);
	void setMessageHandler(NymphMessageHandler handler);
	bool shutdown();
	bool connect(std::string host, int port, int &handle, void* data, std::string &result);
	bool connect(std::string url, int &handle, void* data, std::string &result);
	bool connect(Poco::Net::SocketAddress sa, int &handle, void* data, std::string &result);
	bool disconnect(int handle, std::string &result);
	
	void setWill(std::string will);
	//bool connect(std::string host, int port);
	bool publish(int handle, std::string topic, std::string payload, std::string &result, 
					MqttQoS qos = MQTT_QOS_AT_MOST_ONCE, bool retain = false);
	bool subscribe(int handle, std::string topic, std::string result);
};


#endif
