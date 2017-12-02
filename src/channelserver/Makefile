CC = g++

LFLAGS = -m64 -pthread -lrt
CFLAGS = -c -fPIC

PF = LINUX

ifneq ($(shell uname), Linux) 
PF = MACOS
endif

CFLAGS += -D$(PF)

ifdef RELEASE
	CFLAGS += -Wall -Werror -g -O2
	LFLAGS += -Wall -Werror -g -O2
else
	CFLAGS += -g -D_DEBUG
	LFLAGS += -g
endif
CFLAGS += -I.
CFLAGS += -I./../common/
CFLAGS += -I./$(RAKNET_SOURCE_DIR)/Source
CFLAGS += -I/usr/local
CFLAGS += -std=c++0x

RAKNET_SOURCE_DIR = ../../3rd/RakNet

COMMON_LIB = ../lib/libcommon.a

TARGET = ../bin/channelserver
SOURCES = $(wildcard *.cpp)
OBJS = $(patsubst %.cpp,objs/%.o,$(SOURCES)) $(COMMON_LIB)
#DEPS = $(patsubst %.cpp,objs/%.d,$(SOURCES))


default : $(TARGET)

objs/%.o: %.cpp
	$(CC) $(CFLAGS) -o $@ $<

#-include $(DEPS)

clean :
	rm -f $(TARGET) objs/*.o

$(TARGET) : $(TEMP) $(OBJS)
	$(CC) $(LFLAGS) -o $@ $(OBJS) 
	