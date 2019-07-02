/*
	request.h - header file for the Request class.
	
	Revision 0
	
	Notes:
			- 
			
	2016/11/19, Maya Posch
	(c) Nyanko.ws
*/


#ifndef REQUEST_H
#define REQUEST_H


#include "abstract_request.h"
#include "connections.h"
#include "message.h"


#include <string>
#include <atomic>
#include <condition_variable>
#include <mutex>

using namespace std;


/* class Semaphore {
	static std::atomic<int> lvl;
	static std::condition_variable* cv;
	static std::mutex* mtx;

public:
	static void init(int n, std::condition_variable* cv, std::mutex* mtx);
	static void wait() { --Semaphore::lvl; }
	static void signal();
	static bool processing() { return (Semaphore::lvl == 0)? true : false; }
}; */


//typedef void (*logFunction)(string text);


class Request : public AbstractRequest {
	std::string value;
	int handle;
	NmqttMessage msg;
	std::string loggerName = "Request";
	
	//logFunction outFnc;
	
public:
	Request() { }
	void setValue(std::string value) { this->value = value; }
	void setMessage(int handle, NmqttMessage msg) { this->handle = handle; this->msg = msg; }
	//void setOutput(logFunction fnc) { outFnc = fnc; }
	void process();
	void finish();
};

#endif
