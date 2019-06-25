# Makefile for the NymphMQTT MQTT library.



SOURCES := $(wildcard cpp/*.cpp)
OBJECTS := $(addprefix obj/,$(notdir) $(SOURCES:.cpp=.o))
LIBS := -lPocoNet -lPocoUtil -lPocoFoundation -lPocoJSON

# Check for MinGW and patch up POCO
# The OS variable is only set on Windows.
ifdef OS
	CFLAGS := $(CFLAGS) -U__STRICT_ANSI__
	#LIBS += -lws2_32
endif

all: build_tests


build_tests: message_parse
	
message_parse:	
	g++ -o message_parse_test cpp-test/message_parse_test.cpp ../../endianness/src/cpp/src/bytebauble.cpp cpp/message.cpp -I cpp/ -I ../../endianness/src/cpp/src -g3
	
publish_message:
	g++ -o message_publish_test cpp-test/message_publish_test.cpp ../../endianness/src/cpp/src/bytebauble.cpp cpp/message.cpp -I cpp/ -I ../../endianness/src/cpp/src -g3
	
subscribe_broker:
	g++ -o client_broker_test cpp-test/client_broker_test.cpp ../../endianness/src/cpp/src/bytebauble.cpp $(SOURCES) -I cpp/ -I ../../endianness/src/cpp/src -g3 $(LIBS)
	