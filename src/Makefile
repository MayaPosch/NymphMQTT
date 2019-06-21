# Makefile for the NymphMQTT MQTT library.


all: build_tests


build_tests:
	g++ -o message_parse_test cpp-test/message_parse_test.cpp ../../endianness/src/cpp/src/bytebauble.cpp cpp/message.cpp -I cpp/ -I ../../endianness/src/cpp/src -g3
	