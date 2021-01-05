/*
	server_connections.cpp - Implementation of the NymphMQTT Server Connections class.
	
	Revision 0
	
	Features:
			- Static class to enable the global management of client connections.
			
	Notes:
			- 
			
	2021/01/04 - Maya Posch
*/


#include "server_connections.h"


// Static initialisations.
std::map<uint64_t, NmqttClientSocket> NmqttClientConnections::sockets;
uint64_t NmqttClientConnections::lastHandle = 0;
std::queue<uint64_t> NmqttClientConnections::freeHandles;
NmqttClientSocket NmqttClientConnections::coreCS;


// --- ADD SOCKET ---
uint64_t NmqttClientConnections::addSocket(NmqttClientSocket &ns) {
	// Add new instance to map, return either a new handle ID, or one from the FIFO.
	uint64_t handle;
	if (!freeHandles.empty()) {
		handle = freeHandles.front();
		freeHandles.pop();
	}
	else {
		handle = lastHandle++;
	}
	
	// Merge core and provided struct.
	NmqttClientSocket ts = coreCS;
	ts.socket = ns.socket;
	sockets.insert(std::pair<uint64_t, NmqttClientSocket>(handle, ts));
	
	return handle;
}


// --- GET SOCKET ---
NmqttClientSocket* NmqttClientConnections::getSocket(uint64_t handle) {
	std::map<uint64_t, NmqttClientSocket>::iterator it;
	it = sockets.find(handle);
	if (it == sockets.end()) {
		return 0;
	}
	
	return &it->second;
}


// --- REMOVE SOCKET ---
void NmqttClientConnections::removeSocket(uint64_t handle) {
	std::map<uint64_t, NmqttClientSocket>::iterator it;
	it = sockets.find(handle);
	if (it == sockets.end()) {
		return;
	}
	
	sockets.erase(it);
	freeHandles.push(handle);
}


void NmqttClientConnections::setCoreParameters(NmqttClientSocket &ns) {
	coreCS = ns;
}
