/*
	timer.h - Header for the NymphMQTT Timer class.
	
	Revision 0
	
	Features:
			- Allows one to set custom data to be passed along with the Poco Timer.
			
	Notes:
			- 
			
	2019/05/08 - Maya Posch
*/


#ifndef NMQTT_TIMER_H
#define NMQTT_TIMER_H


#include <Poco/Timer.h>


class NmqttTimer : public Poco::Timer {
	int handle;
	
public:
	NmqttTimer(long startInterval = 0, long periodicInterval = 0) : Poco::Timer(startInterval, periodicInterval) { }
	void setHandle(int handle) { this->handle = handle; }
	int getHandle() { return handle; }
};


#endif
