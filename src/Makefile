# Makefile for the NymphMQTT MQTT library.

GCC := g++
MKDIR := mkdir -p
RM = rm
AR = ar

OUTPUT := libnymphmqtt.a

SOURCES := $(wildcard cpp/*.cpp)
OBJECTS := $(addprefix obj/,$(notdir) $(SOURCES:.cpp=.o))
LIBS := -lPocoNet -lPocoNetSSL -lPocoUtil -lPocoFoundation -lPocoJSON -lbytebauble
INCLUDES := -I cpp/ -I cpp/ 
CFLAGS := -std=c++11 -g3 -O0 -pthread

# Check for MinGW and patch up POCO
# The OS variable is only set on Windows.
ifdef OS
	CFLAGS := $(CFLAGS) -U__STRICT_ANSI__
	#LIBS += -lws2_32
endif

all: lib build_tests

lib: makedir $(OBJECTS) lib/$(OUTPUT)

test: lib build_tests

makedir:
	$(MKDIR) bin
	$(MKDIR) lib
	$(MKDIR) obj/cpp
	
obj/%.o: %.cpp
	$(GCC) -c -o $@ $< $(CFLAGS) $(INCLUDES)
	
lib/$(OUTPUT): $(OBJECTS)
	-rm -f $@
	$(AR) rcs $@ $^

build_tests: message_parse publish_message subscribe_broker
	
message_parse:	
	g++ -o bin/message_parse_test cpp-test/message_parse_test.cpp $(OBJECTS) $(INCLUDES) $(CFLAGS) $(LIBS)
	
publish_message:
	g++ -o bin/message_publish_test cpp-test/message_publish_test.cpp $(OBJECTS) $(INCLUDES) -$(CFLAGS) $(LIBS)
	
subscribe_broker:
	g++ -o bin/client_broker_test cpp-test/client_broker_test.cpp $(OBJECTS)  $(INCLUDES) $(CFLAGS) $(LIBS)
	
clean:
	rm $(OBJECTS)
