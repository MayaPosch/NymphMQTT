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
#include <functional>

#include <Poco/Mutex.h>
#include <Poco/Semaphore.h>
#include <Poco/Net/SocketAddress.h>
#include <Poco/Net/StreamSocket.h>
#include <Poco/Condition.h>

#include "nymph_logger.h"
#include "message.h"
#include "chronotrigger.h"


struct NmqttBrokerConnection {
	int handle;
	std::string host;
	int port;
	bool sessionPresent;
	MqttReasonCodes responseCode;
};


class NmqttClient {
	std::map<int, Poco::Net::StreamSocket*> sockets;
	std::map<int, Poco::Semaphore*> socketSemaphores;
	Poco::Mutex socketsMutex;
	int lastHandle = 0;
	long timeout = 3000;
	std::string loggerName = "NmqttClient";
	std::function<void(int, std::string, std::string)> messageHandler;
	Poco::Condition connectCnd;
	Poco::Mutex connectMtx;
	ChronoTrigger pingTimer;
	NmqttBrokerConnection* brokerConn = 0;
	bool secureConnection = false;
	
	uint8_t connectFlags;
	bool cleanSessionFlag = true;
	bool willFlag = false;
	bool willRetainFlag = false;
	bool usernameFlag = false;
	bool passwordFlag = false;
	uint8_t willQoS = 0;
	std::string will;
	std::string willTopic;
	std::string clientId = "NymphMQTT-client";
	std::string username;
	std::string password;
	std::string ca, cert, key;
	
	bool sendMessage(int handle, std::string binMsg);
	void connackHandler(int handle, bool sessionPresent, MqttReasonCodes code);
	void pingreqHandler(uint32_t t);
	void pingrespHandler(int handle);
	
public:
	NmqttClient();
	
	bool init(std::function<void(int, std::string)> logger, int level = NYMPH_LOG_LEVEL_TRACE, long timeout = 3000);
	void setLogger(std::function<void(int, std::string)> logger, int level);
	void setMessageHandler(std::function<void(int, std::string, std::string)> handler);
	bool shutdown();
	bool connect(std::string host, int port, int &handle, void* data, 
					NmqttBrokerConnection &conn, std::string &result);
	bool connect(std::string url, int &handle, void* data, 
					NmqttBrokerConnection &conn, std::string &result);
	bool connect(Poco::Net::SocketAddress sa, int &handle, void* data, 
					NmqttBrokerConnection &conn, std::string &result);
	bool disconnect(int handle, std::string &result);
	
	void setCredentials(std::string &user, std::string &pass);
	void setWill(std::string topic, std::string will, uint8_t qos = 0, bool retain = false);
	void setTLS(std::string &ca, std::string &cert, std::string &key);
	void setClientId(std::string id) { clientId = id; }
	bool publish(int handle, std::string topic, std::string payload, std::string &result, 
					MqttQoS qos = MQTT_QOS_AT_MOST_ONCE, bool retain = false);
	bool subscribe(int handle, std::string topic, std::string result);
	bool unsubscribe(int handle, std::string topic, std::string result);
	
	std::string getLocalAddress(int handle);
};


#endif
