# Makefile for the NymphMQTT MQTT library.

GCC := g++
MKDIR := mkdir -p

SOURCES := $(wildcard cpp/*.cpp)
OBJECTS := $(addprefix obj/,$(notdir) $(SOURCES:.cpp=.o))
LIBS := -lPocoNet -lPocoUtil -lPocoFoundation -lPocoJSON
INCLUDES := -I cpp/ -I cpp/ -I ../../ByteBauble/src/cpp/src
CFLAGS := -std=c++11 -g3 -O0 -pthread

# Check for MinGW and patch up POCO
# The OS variable is only set on Windows.
ifdef OS
	CFLAGS := $(CFLAGS) -U__STRICT_ANSI__
	#LIBS += -lws2_32
endif

all: makedir $(OBJECTS) build_tests

makedir:
	$(MKDIR) bin
	$(MKDIR) obj/cpp
	
obj/%.o: %.cpp
	$(GCC) -c -o $@ $< $(CFLAGS) $(INCLUDES)

build_tests: message_parse publish_message subscribe_broker
	
message_parse:	
	g++ -o bin/message_parse_test cpp-test/message_parse_test.cpp ../../ByteBauble/src/cpp/src/bytebauble.cpp $(OBJECTS) $(INCLUDES) $(CFLAGS) $(LIBS)
	
publish_message:
	g++ -o bin/message_publish_test cpp-test/message_publish_test.cpp ../../ByteBauble/src/cpp/src/bytebauble.cpp $(OBJECTS) $(INCLUDES) -$(CFLAGS) $(LIBS)
	
subscribe_broker:
	g++ -o bin/client_broker_test cpp-test/client_broker_test.cpp ../../ByteBauble/src/cpp/src/bytebauble.cpp $(OBJECTS)  $(INCLUDES) $(CFLAGS) $(LIBS)
	