# Makefile for the NymphMQTT MQTT library.


all: build_tests


build_tests: message_parse
	
message_parse:	
	g++ -o message_parse_test cpp-test/message_parse_test.cpp ../../endianness/src/cpp/src/bytebauble.cpp cpp/message.cpp -I cpp/ -I ../../endianness/src/cpp/src -g3
	
publish_message:
	g++ -o message_publish_test cpp-test/message_publish_test.cpp ../../endianness/src/cpp/src/bytebauble.cpp cpp/message.cpp -I cpp/ -I ../../endianness/src/cpp/src -g3
	