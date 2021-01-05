/*
	session.h - Declaration of NMQTT session class.
	
	2021/01/04, Maya Posch
*/


#ifndef NMQTT_SESSION_H
#define NMQTT_SESSION_H


#include <Poco/Net/TCPServerConnection.h>
#include <Poco/Semaphore.h>
#include <Poco/Condition.h>


class NmqttSession : public Poco::Net::TCPServerConnection {
	std::string loggerName;
	bool listen;
	bool init;
	Poco::Condition* readyCond;
	Poco::Mutex* readyMutex;
	
public:
	NmqttSession(const Poco::Net::StreamSocket& socket);
	~NmqttSession();
	void run();
	void stop();
};

#endif
