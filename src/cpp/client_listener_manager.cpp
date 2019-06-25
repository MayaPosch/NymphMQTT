/*
	client_listener_manager.cpp	- Declares the NymphMQTT Listener Manager class.
	
	Revision 0
	
	Notes:
			- 
			
	History:
	2019/06/24, Maya Posch	: Initial version.
	
	(c) Nyanko.ws
*/


#include "client_listener_manager.h"
#include "client_listener.h"
#include "nymph_logger.h"

#include <iostream>
#include <cstdlib>

using namespace std;

#include <Poco/Thread.h>
#include <Poco/NumberFormatter.h>
#include <Poco/Condition.h>

using namespace Poco;


// Static initialisations.
map<int, NmqttClientListener*> NmqttClientListenerManager::listeners;
Mutex NmqttClientListenerManager::listenersMutex;
string NmqttClientListenerManager::loggerName = "NmqttClientListenerManager";


// --- STOP ---
void NmqttClientListenerManager::stop() {
	// Shut down all listening threads.
	listenersMutex.lock();
	for (int i = 0; i < listeners.size(); ++i) {
		listeners[i]->stop();
	}
	
	listenersMutex.unlock();
}


// --- ADD CONNECTION ---
bool NmqttClientListenerManager::addConnection(int handle, NymphSocket socket) {
	NYMPH_LOG_INFORMATION("Adding connection. Handle: " + NumberFormatter::format(handle) + ".");
	
	// Create new thread for NmqttListener instance which handles
	// the new socket. Save reference to this listener.
	Poco::Condition* cnd = new Poco::Condition;
	Poco::Mutex* mtx = new Poco::Mutex;
	long timeout = 1000; // 1 second
	mtx->lock();
	NmqttClientListener* esl = new NmqttClientListener(socket, cnd, mtx);
	Poco::Thread* thread = new Poco::Thread;
	thread->start(*esl);
	if (!cnd->tryWait(*mtx, timeout)) {
		// Handle listener timeout.
		NYMPH_LOG_ERROR("Creating of new listener thread timed out.");
		mtx->unlock();
		return false;
	}
	
	mtx->unlock();
	
	listenersMutex.lock();
	listeners.insert(std::pair<int, NmqttClientListener*>(handle, esl));
	listenersMutex.unlock();
	
	NYMPH_LOG_INFORMATION("Listening socket has been added.");
	
	return true;
}


// --- REMOVE CONNECTION ---
// Removes a connection using the Nymph connection handle.
bool NmqttClientListenerManager::removeConnection(int handle) {
	map<int, NmqttClientListener*>::iterator it;
	listenersMutex.lock();
	
	NYMPH_LOG_INFORMATION("Removing connection for handle: " + NumberFormatter::format(handle) + ".");
	
	it = listeners.find(handle);
	if (it == listeners.end()) { listenersMutex.unlock(); return true; }
	
	it->second->stop(); // Tell the listening thread to terminate.
	listeners.erase(it);
	
	NYMPH_LOG_INFORMATION("Listening socket has been removed.");
	
	listenersMutex.unlock();
	return true;
}

