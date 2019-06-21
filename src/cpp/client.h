/*
	client.h - Header for the NymphMQTT Client class.
	
	Revision 0
	
	Features:
			- 
			
	Notes:
			- 
			
	2019/05/08 - Maya Posch
*/


#include <string>


class NmqttClient {
	//
	std::string will;
	
public:
	NmqttClient();
	
	void setWill(std::string will);
	bool connect(std::string host, int port);
	bool publish(std::string topic, std::string payload, int qos = 0, bool retain = false);
};
