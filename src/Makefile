# Makefile for the NymphMQTT MQTT library.

export TOP := $(CURDIR)

ifndef ANDROID_ABI_LEVEL
ANDROID_ABI_LEVEL := 24
endif

ifdef ANDROID
TOOLCHAIN_PREFIX := arm-linux-androideabi-
ARCH := android-armv7/
ifdef OS
TOOLCHAIN_POSTFIX := .cmd
endif
else ifdef ANDROID64
TOOLCHAIN_PREFIX := aarch64-linux-android-
ARCH := android-aarch64/
ifdef OS
TOOLCHAIN_POSTFIX := .cmd
endif
else ifdef ANDROIDX86
TOOLCHAIN_PREFIX := i686-linux-android-
ARCH := android-i686/
ifdef OS
TOOLCHAIN_POSTFIX := .cmd
endif
else ifdef ANDROIDX64
TOOLCHAIN_PREFIX := x86_64-linux-android-
ARCH := android-x86_64/
ifdef OS
TOOLCHAIN_POSTFIX := .cmd
endif
endif

ifndef ARCH
ARCH := $(shell g++ -dumpmachine)/
endif

USYS := $(shell uname -s)
UMCH := $(shell uname -m)

ifdef ANDROID
#GCC := $(TOOLCHAIN_PREFIX)g++$(TOOLCHAIN_POSTFIX)
GCC := armv7a-linux-androideabi$(ANDROID_ABI_LEVEL)-clang++$(TOOLCHAIN_POSTFIX)
MAKEDIR = mkdir -p
RM = rm
AR = $(TOOLCHAIN_PREFIX)ar
else ifdef ANDROID64
GCC := aarch64-linux-android$(ANDROID_ABI_LEVEL)-clang++$(TOOLCHAIN_POSTFIX)
MAKEDIR = mkdir -p
RM = rm
AR = $(TOOLCHAIN_PREFIX)ar
else ifdef ANDROIDX86
GCC := i686-linux-android$(ANDROID_ABI_LEVEL)-clang++$(TOOLCHAIN_POSTFIX)
MAKEDIR = mkdir -p
RM = rm
AR = $(TOOLCHAIN_PREFIX)ar
else ifdef ANDROIDX64
GCC := x86_64-linux-android$(ANDROID_ABI_LEVEL)-clang++$(TOOLCHAIN_POSTFIX)
MAKEDIR = mkdir -p
RM = rm
AR = $(TOOLCHAIN_PREFIX)ar
else ifdef WASM
GCC = emc++
MAKEDIR = mkdir -p
RM = rm
AR = ar 
else
GCC = g++
MAKEDIR = mkdir -p
RM = rm
AR = ar
endif


OUTPUT = libnymphmqtt
VERSION = 0.1

# Use -soname on Linux/BSD, -install_name on Darwin (MacOS).
SONAME = -soname
LIBNAME = $(OUTPUT).so.$(VERSION)
ifeq ($(shell uname -s),Darwin)
	SONAME = -install_name
	LIBNAME = $(OUTPUT).0.dylib
endif

#OUTPUT := libnymphmqtt.a
SERVER := nmqtt_server

SOURCES := $(wildcard cpp/*.cpp)
OBJECTS := $(addprefix obj/static/$(ARCH),$(notdir) $(SOURCES:.cpp=.o))
SHARED_OBJECTS := $(addprefix obj/shared/$(ARCH),$(notdir) $(SOURCES:.cpp=.o))

SERVER_SOURCES := $(wildcard server/*.cpp)
SERVER_OBJECTS := $(addprefix obj/,$(notdir) $(SERVER_SOURCES:.cpp=.o))

LIBS := -lPocoNet -lPocoNetSSL -lPocoUtil -lPocoFoundation -lPocoJSON 
#-lbytebauble
INCLUDES := -I cpp/ 
CFLAGS := -std=c++11 -g3 -O0
SHARED_FLAGS := -fPIC -shared -Wl,$(SONAME),$(LIBNAME)

ifdef ANDROID
CFLAGS += -fPIC
else ifdef ANDROID64
#CFLAGS += -fPIC
else ifdef ANDROIDX86
CFLAGS += -fPIC
else ifdef ANDROIDX64
CFLAGS += -fPIC
endif

# Check for MinGW and patch up POCO
# The OS variable is only set on Windows.
ifdef OS
ifndef ANDROID
ifndef ANDROID64
ifndef ANDROIDX86
ifndef ANDROIDX64
	CFLAGS := $(CFLAGS) -U__STRICT_ANSI__ -DPOCO_WIN32_UTF8
	LIBS += -lws2_32
endif
endif
endif
endif
else
	LIBS += -pthread
	UNAME_S := $(shell uname -s)
    ifeq ($(UNAME_S),Linux)
        MAKE := gmake
    endif
endif

all: makedir lib

lib: $(OBJECTS) lib/$(ARCH)$(OUTPUT).a lib/$(ARCH)$(LIBNAME)

test: lib build_tests

makedir:
	$(MAKEDIR) bin
	$(MAKEDIR) lib/$(ARCH)
	$(MAKEDIR) obj/static/$(ARCH)cpp
	$(MAKEDIR) obj/shared/$(ARCH)cpp
	$(MAKEDIR) obj/server
	
obj/static/$(ARCH)%.o: %.cpp
	$(GCC) -c -o $@ $< $(CFLAGS)
	
obj/shared/$(ARCH)%.o: %.cpp
	$(GCC) -c -o $@ $< $(SHARED_FLAGS) $(CFLAGS) $(LIBS)
	
lib/$(ARCH)$(OUTPUT).a: $(OBJECTS)
	-rm -f $@
	$(AR) rcs $@ $^
	
lib/$(ARCH)$(LIBNAME): $(SHARED_OBJECTS)
	$(GCC) -o $@ $(CFLAGS) $(SHARED_FLAGS) $(SHARED_OBJECTS) $(LIBS)
	
server: lib $(SERVER_OBJECTS)
	$(GCC) -o bin/$(SERVER) $(OBJECTS) $(SERVER_OBJECTS) $(CFLAGS) $(LIBS) $(INCLUDES)

build_tests: message_parse publish_message subscribe_broker
	
message_parse:	
	g++ -o bin/message_parse_test cpp-test/message_parse_test.cpp $(OBJECTS) $(INCLUDES) $(CFLAGS) $(LIBS)
	
publish_message:
	g++ -o bin/message_publish_test cpp-test/message_publish_test.cpp $(OBJECTS) $(INCLUDES) -$(CFLAGS) $(LIBS)
	
subscribe_broker:
	g++ -o bin/client_broker_test cpp-test/client_broker_test.cpp $(OBJECTS)  $(INCLUDES) $(CFLAGS) $(LIBS)
	
clean:
	rm $(OBJECTS)

PREFIX ?= /usr
ifdef OS
# Assume 64-bit MSYS2
PREFIX = /mingw64
endif

.PHONY: install lib
install:
	install -d $(DESTDIR)$(PREFIX)/lib/
	install -m 644 lib/$(ARCH)$(OUTPUT).a $(DESTDIR)$(PREFIX)/lib/
ifndef OS
	install -m 644 lib/$(ARCH)$(OUTPUT).so.$(VERSION) $(DESTDIR)$(PREFIX)/lib/
endif
	install -d $(DESTDIR)$(PREFIX)/include/nymphmqtt
	install -m 644 cpp/*.h $(DESTDIR)$(PREFIX)/include/nymphmqtt/
ifndef OS
	cd $(DESTDIR)$(PREFIX)/lib && \
		if [ -f $(OUTPUT).so ]; then \
			rm $(OUTPUT).so; \
		fi && \
		ln -s $(OUTPUT).so.$(VERSION) $(OUTPUT).so
endif
