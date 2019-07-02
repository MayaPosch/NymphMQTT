/*
	connections.cpp - Impplementation of the NymphMQTT Connections class.
	
	Revision 0
	
	Features:
			- Static class to enable the global management of connections.
			
	Notes:
			- 
			
	2019/05/08 - Maya Posch
*/


#include "connections.h"


// Static declarations.
std::map<int, NymphSocket> NmqttConnections::sockets;


void NmqttConnections::addSocket(NymphSocket &ns) {
	sockets.insert(std::pair<int, NymphSocket>(ns.handle, ns));
}


// --- GET SOCKET ---
NymphSocket* NmqttConnections::getSocket(int handle) {
	std::map<int, NymphSocket>::iterator it;
	it = sockets.find(handle);
	if (it == sockets.end()) {
		return 0;
	}
	
	return &it->second;
}
