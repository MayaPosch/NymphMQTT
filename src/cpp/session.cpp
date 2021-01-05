/*
	session.cpp - Definition of the NMQTT Session class.
	
	Features:
			- Handles client connections.
			
	2021/01/04, Maya Posch
*/


#include "session.h"

#include "server_connections.h"
#include "server_request.h"
#include "dispatcher.h"

#include <Poco/NumberFormatter.h>


// --- CONSTRUCTOR ---
NmqttSession::NmqttSession(const Poco::Net::StreamSocket& socket) 
							: Poco::Net::TCPServerConnection(socket) {
	loggerName = "NmqttSession";
	listen = true;
}


// --- CONSTRUCTOR ---
/* NmqttSession::NmqttSession(int handle, Condition* cnd, Mutex* mtx) {
	loggerName = "NmqttSession";
	listen = true;
	init = true;
	this->nymphSocket = NmqttConnections::getSocket(handle);
	this->socket = nymphsocket.socket;
	this->readyCond = cnd;
	this->readyMutex = mtx;
} */


// --- DECONSTRUCTOR ---
NmqttSession::~NmqttSession() {
	//
}


// --- RUN ---
void NmqttSession::run() {
	Poco::Net::StreamSocket& socket = this->socket();
	
	// Add this connection to the list of client connections.
	NmqttClientSocket sk;
	sk.socket = &socket;
	uint64_t handle = NmqttClientConnections::addSocket(sk);
	
	Poco::Timespan timeout(0, 100); // 100 microsecond timeout
	
	NYMPH_LOG_INFORMATION("Start listening...");
	
	char headerBuff[5];
	while (listen) {
		if (socket.poll(timeout, Poco::Net::Socket::SELECT_READ)) {
			// Attempt to receive the entire message.
			// First validate the first two bytes. If it's an MQTT message this will contain the
			// command and the first byte of the message length.
			//
			// Unfortunately, MQTT's message length is a variable length integer, spanning 1-4 bytes.
			// Because of this, we have to read in the first byte, see whether a second one follows
			// by looking at the 8th bit of the byte, read that in, and so on.
			//
			// The smallest message we can receive is the Disconnect type, with just two bytes.
			int received = socket.receiveBytes((void*) &headerBuff, 2);
			if (received == 0) {
				// Remote disconnnected. Socket should be discarded.
				NYMPH_LOG_INFORMATION("Received remote disconnected notice. Terminating listener thread.");
				break;
			}
			else if (received < 2) {
				// TODO: try to wait for more bytes.
				NYMPH_LOG_WARNING("Received <2 bytes: " + Poco::NumberFormatter::format(received));
				
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
			
			NYMPH_LOG_DEBUG("Message length: " + Poco::NumberFormatter::format(msglen));
			
			std::string binMsg;
			if (msglen > 0) {
				// Create new buffer for the rest of the message.
				char* buff = new char[msglen];
				
				// Read the entire message into a string which is then used to
				// construct an NmqttMessage instance.
				received = socket.receiveBytes((void*) buff, msglen);
				binMsg.append(headerBuff, idx);
				binMsg.append(buff, received);
				if (received != msglen) {
					// Handle incomplete message.
					NYMPH_LOG_WARNING("Incomplete message: " 
										+ Poco::NumberFormatter::format(received) 
										+ " of " + Poco::NumberFormatter::format(msglen));
					
					// Loop until the rest of the message has been received.
					// TODO: Set a maximum number of loops/timeout? Reset when 
					// receiving data, timeout when poll times out N times?
					//binMsg->reserve(msglen);
					int unread = msglen - received;
					while (1) {
						if (socket.poll(timeout, Poco::Net::Socket::SELECT_READ)) {
							char* buff1 = new char[unread];
							received = socket.receiveBytes((void*) buff1, unread);
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
								NYMPH_LOG_WARNING("Incomplete message: " 
													+ Poco::NumberFormatter::format(unread) + "/" 
													+ Poco::NumberFormatter::format(msglen) 
													+ " unread.");
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
					NYMPH_LOG_DEBUG("Read " + Poco::NumberFormatter::format(received) + " bytes.");
				}
				
				delete[] buff;
			}
			else {
				//
				binMsg.append(headerBuff, idx);
			}
			
			// Parse the string into an NmqttMessage instance.
			msg.parseMessage(binMsg);	
			
			NYMPH_LOG_DEBUG("Got command: " + Poco::NumberFormatter::format(msg.getCommand()));
			
			// Call the message handler callback when one exists for this type of message.
			// TODO: refactor for Dispatcher.
			NmqttServerRequest* req = new NmqttServerRequest;
			req->setMessage(handle, msg);
			Dispatcher::addRequest(req);
		}
		
		// Check whether we're still initialising.
		/* if (init) {
			// Signal that this listener thread is ready.
			readyMutex->lock();
			readyCond->signal();
			readyMutex->unlock();
			
			timeout.assign(1, 0); // Change timeout to 1 second.
			init = false;
		} */
	}
	
	NYMPH_LOG_INFORMATION("Stopping thread...");
	
	// Clean-up.
	// Remove this session from the list.
	NmqttClientConnections::removeSocket(handle);
}


// --- STOP ---
void NmqttSession::stop() {
	listen = false;
}
