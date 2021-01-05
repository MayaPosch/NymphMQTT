/*
	server_request.cpp - implementation of the Server Request class.
	
	Revision 0
	
	Notes:
			- 
			
	2021/01/04, Maya Posch
	(c) Nyanko.ws
*/


#include "server_request.h"

#include "nymph_logger.h"


// --- PROCESS ---
void NmqttServerRequest::process() {
	NmqttClientSocket* clientSocket = NmqttClientConnections::getSocket(handle);
	
	/* if (msg.getCommand() == MQTT_PUBLISH) {
		NYMPH_LOG_DEBUG("Calling PUBLISH message handler...");
		clientSocket->handler(handle, msg.getTopic(), msg.getPayload());
	}
	else */ if (msg.getCommand() == MQTT_CONNECT) {
		NYMPH_LOG_DEBUG("Calling CONNECT message handler...");
		clientSocket->connectHandler(handle);
	}
	else if (msg.getCommand() == MQTT_PINGREQ) {
		NYMPH_LOG_DEBUG("Calling PINGREQ message handler...");
		clientSocket->pingreqHandler(handle);
	}
}


// --- FINISH ---
void NmqttServerRequest::finish() {
	//
}
