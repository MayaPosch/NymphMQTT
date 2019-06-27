/*
	client_listener.h - Header for the NymphMQTT Client socket listening thread class.
	
	Revision 0
	
	Features:
			- Socket listener for MQTT connections.
			
	Notes:
			- 
			
	2019/05/08 - Maya Posch
*/


#include "client_listener.h"
#include "message.h"
#include "nymph_logger.h"

using namespace std;

#include <Poco/NumberFormatter.h>

using namespace Poco;


// --- CONSTRUCTOR ---
NmqttClientListener::NmqttClientListener(NymphSocket socket, Condition* cnd, Mutex* mtx) {
	loggerName = "NmqttClientListener";
	listen = true;
	init = true;
	this->nymphSocket = socket;
	this->socket = socket.socket;
	this->readyCond = cnd;
	this->readyMutex = mtx;
}


// --- DECONSTRUCTOR ---
NmqttClientListener::~NmqttClientListener() {
	//
}


// --- RUN ---
void NmqttClientListener::run() {
	Poco::Timespan timeout(0, 100); // 100 microsecond timeout
	
	NYMPH_LOG_INFORMATION("Start listening...");
	
	char headerBuff[5];
	while (listen) {
		if (socket->poll(timeout, Net::Socket::SELECT_READ)) {
			// Attempt to receive the entire message.
			// First validate the first two bytes. If it's an MQTT message this will contain the
			// command and the first byte of the message length.
			//
			// Unfortunately, MQTT's message length is a variable length integer, spanning 1-4 bytes.
			// Because of this, we have to read in the first byte, see whether a second one follows
			// by looking at the 8th bit of the byte, read that in, and so on.
			//
			// The smallest message we can receive is the Disconnect type, with just two bytes.
			int received = socket->receiveBytes((void*) &headerBuff, 2);
			if (received == 0) {
				// Remote disconnnected. Socket should be discarded.
				NYMPH_LOG_INFORMATION("Received remote disconnected notice. Terminating listener thread.");
				break;
			}
			else if (received < 2) {
				// TODO: try to wait for more bytes.
				NYMPH_LOG_WARNING("Received <2 bytes: " + NumberFormatter::format(received));
				
				continue;
			}
			
			// Use the NmqttMessage class's validation feature to extract the message length from
			// the fixed header.
			NmqttMessage msg;
			uint32_t msglen = 0;
			int idx = 0; // Will be set to the index after the fixed header by the parse method.
			while (!msg.parseHeader((char*) &headerBuff, 5, msglen, idx)) {
				// Attempt to read more data. The index parameter is placed at the last valid
				// byte in the headerBuff array. Append new data after this.
				
				NYMPH_LOG_WARNING("Too few bytes to parse header. Aborting...");
				
				// TODO: abort reading for now.
				continue;
			}
			
			NYMPH_LOG_DEBUG("Message length: 0x" + NumberFormatter::formatHex(msglen));
			
			// Create new buffer for the rest of the message.
			char* buff = new char[msglen];
			
			// Read the entire message into a string which is then used to
			// construct an NymphMessage instance.
			received = socket->receiveBytes((void*) buff, msglen);
			string binMsg;
			binMsg.append(headerBuff, idx);
			binMsg.append(buff, received);
			if (received != msglen) {
				// Handle incomplete message.
				NYMPH_LOG_WARNING("Incomplete message: " + NumberFormatter::format(received) + " of " + NumberFormatter::format(msglen));
				
				// Loop until the rest of the message has been received.
				// TODO: Set a maximum number of loops/timeout? Reset when 
				// receiving data, timeout when poll times out N times?
				//binMsg->reserve(msglen);
				int unread = msglen - received;
				while (1) {
					if (socket->poll(timeout, Net::Socket::SELECT_READ)) {
						char* buff1 = new char[unread];
						received = socket->receiveBytes((void*) buff1, unread);
						if (received == 0) {
							// Remote disconnnected. Socket should be discarded.
							NYMPH_LOG_INFORMATION("Received remote disconnected notice. Terminating listener thread.");
							delete[] buff1;
							break;
						}
						else if (received != unread) {
							binMsg.append((const char*) buff1, received);
							delete[] buff1;
							unread -= received;
							NYMPH_LOG_WARNING("Incomplete message: " + NumberFormatter::format(unread) + "/" + NumberFormatter::format(msglen) + " unread.");
							continue;
						}
						
						// Full message was read. Continue with processing.
						binMsg.append((const char*) buff1, received);
						delete[] buff1;
						break;
					} // if
				} //while
			}
			else { 
				NYMPH_LOG_DEBUG("Read 0x" + NumberFormatter::formatHex(received) + " bytes.");
			}
			
			delete[] buff;
			
			// Parse the string into an NymphMessage instance.
			msg.parseMessage(binMsg);	
			
			// Call the message handler callback when it's a publish message we got.
			if (msg.getCommand() == MQTT_PUBLISH) {
				NYMPH_LOG_DEBUG("Calling publish message handler...");
				nymphSocket.handler(nymphSocket.handle, msg.getTopic(), msg.getPayload());
			}
		}
		
		// Check whether we're still initialising.
		if (init) {
			// Signal that this listener thread is ready.
			readyMutex->lock();
			readyCond->signal();
			readyMutex->unlock();
			
			timeout.assign(1, 0); // Change timeout to 1 second.
			init = false;
		}
	}
	
	NYMPH_LOG_INFORMATION("Stopping thread...");
	
	// Clean-up.
	delete readyCond;
	delete readyMutex;
	nymphSocket.semaphore->wait();	// Wait for the connection to be closed.
	delete socket;
	delete nymphSocket.semaphore;
	nymphSocket.semaphore = 0;
	delete this; // Call the destructor ourselves.
}


// --- STOP ---
void NmqttClientListener::stop() {
	listen = false;
}

