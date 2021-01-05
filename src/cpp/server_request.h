/*
	server_request.h - header file for the Server Request class.
	
	Revision 0
	
	Notes:
			- 
			
	2021/01/04, Maya Posch
	(c) Nyanko.ws
*/


#ifndef SERVER_REQUEST_H
#define SERVER_REQUEST_H


#include "abstract_request.h"
#include "server_connections.h"
#include "message.h"


#include <string>
#include <atomic>
#include <condition_variable>
#include <mutex>

using namespace std;


class NmqttServerRequest : public AbstractRequest {
	std::string value;
	int handle;
	NmqttMessage msg;
	std::string loggerName = "NmqttServerRequest";
	
public:
	NmqttServerRequest() { }
	void setValue(std::string value) { this->value = value; }
	void setMessage(uint64_t handle, NmqttMessage msg) { this->handle = handle; this->msg = msg; }
	void process();
	void finish();
};

#endif
