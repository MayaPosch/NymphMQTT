/*
	server.h - Declaration of the NymphMQTT server class.
	
	Revision 0.
	
	Features:
			- Provides public API for NymphMQTT server.
			
	Notes:
			- 
			
	2021/01/03, Maya Posch
*/


#ifndef NMQTT_SERVER_H
#define NMQTT_SERVER_H

#include <string>
#include <functional>

#include <Poco/Net/SocketAddress.h>
#include <Poco/Net/StreamSocket.h>
#include <Poco/Net/TCPServer.h>

#include "nymph_logger.h"
#include "message.h"


class NmqttServer {
	static long timeout;
	static std::string loggerName;
	static Poco::Net::ServerSocket ss;
	static Poco::Net::TCPServer* server;
	
	static bool sendMessage(uint64_t handle, std::string binMsg);
	static void connectHandler(uint64_t handle);
	static void pingreqHandler(uint64_t handle);
	
public:
	NmqttServer();
	~NmqttServer();
	
	static bool init(std::function<void(int, std::string)> logger, int level = NYMPH_LOG_LEVEL_TRACE, long timeout = 3000);
	static void setLogger(std::function<void(int, std::string)> logger, int level);
	static bool start(int port = 4004);
	static bool shutdown();
};

#endif
