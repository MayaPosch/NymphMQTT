/*
	request.cpp - implementation of the Request class.
	
	Revision 0
	
	Notes:
			- 
			
	2016/11/19, Maya Posch
	(c) Nyanko.ws
*/


#include "request.h"

#include "nymph_logger.h"




// Static initialisations
/* std::atomic<int> Semaphore::lvl = 0;
std::condition_variable* Semaphore::cv = 0;
std::mutex* Semaphore::mtx = 0;


// --- INIT ---
void Semaphore::init(int n, std::condition_variable* cv, std::mutex* mtx) { 
	Semaphore::lvl = n;
	Semaphore::cv = cv;
	Semaphore::mtx = mtx;
}


// --- SIGNAL ---
void Semaphore::signal() { 
	++Semaphore::lvl;
	
	if (Semaphore::lvl == 0 && cv) {
		cv->notify_one();
	}
} */


// --- PROCESS ---
void Request::process() {
	NymphSocket* nymphSocket = NmqttConnections::getSocket(handle);
	
	if (msg.getCommand() == MQTT_PUBLISH) {
		NYMPH_LOG_DEBUG("Calling PUBLISH message handler...");
		nymphSocket->handler(handle, msg.getTopic(), msg.getPayload());
	}
	else if (msg.getCommand() == MQTT_CONNACK) {
		NYMPH_LOG_DEBUG("Calling CONNACK message handler...");
		nymphSocket->connackHandler(handle, msg.getSessionPresent(), msg.getReasonCode());
	}
	else if (msg.getCommand() == MQTT_PINGRESP) {
		NYMPH_LOG_DEBUG("Calling PINGRESP message handler...");
		nymphSocket->pingrespHandler(handle);
	}
	
	// Signal that we are done.
	//Semaphore::signal();
}


// --- FINISH ---
void Request::finish() {
	// Call own destructor.
	delete this;
}
