CC = g++
AR = ar

EVENT_SOURCE_DIR = ../../3rd/libevent
LOG4CPP_SOURCE_DIR = ../../3rd/log4cpp
RAKNET_SOURCE_DIR = ../../3rd/RakNet

CFLAGS = -c -fPIC
CFLAGS += -I.
CFLAGS += -I./$(RAKNET_SOURCE_DIR)/Source
CFLAGS += -I/usr/local

EVENT_LIB = /usr/local/lib/libevent.a 
EVENT_LIB_CORE = /usr/local/lib/libevent_core.a 
EVENT_LIB_EXT = /usr/local/lib/libevent_extra.a 
LOG_LIB = /usr/local/lib/liblog4cpp.a
RAKNET_LIB = ../../3rd/RakNet/Lib/libraknet.a

EVENT_LIB_PREFIX = --prefix=/usr/local
LOG_LIB_PREFIX = --prefix=/usr/local

TARGET = ../lib/libcommon.a
SOURCES = $(wildcard *.cpp) $(wildcard json/*.cpp)
OBJS    = $(patsubst %.cpp,objs/%.o,$(SOURCES))

PF = LINUX
DEPS_LIB = $(LOG_LIB) $(EVENT_LIB) $(RAKNET_LIB) 

ifneq ($(shell uname), Linux) 
	PF = MACOS
	EVENT_LIB_PREFIX += CPPFLAGS="-I/usr/local/Cellar/openssl/1.0.2h_1/include" LDFLAGS="-I/usr/local/Cellar/openssl/1.0.2h_1/lib"
endif

CFLAGS += -D$(PF)

ifdef RELEASE
	CFLAGS += -Wall -Werror -g -O2 
else
	CFLAGS += -Wall -Werror -g -D_DEBUG 
endif





default : $(TARGET)

#libevent
$(EVENT_LIB) : $(EVENT_SOURCE_DIR)/Makefile
	$(MAKE) -C $(EVENT_SOURCE_DIR)
	$(MAKE) -C $(EVENT_SOURCE_DIR) install

$(EVENT_SOURCE_DIR)/Makefile : $(EVENT_SOURCE_DIR)/configure
	$(EVENT_SOURCE_DIR)/configure --prefix=/usr/local

#log4cpp
$(LOG_LIB) : $(LOG4CPP_SOURCE_DIR)/Makefile
	$(MAKE) -C $(LOG4CPP_SOURCE_DIR)
	$(MAKE) -C $(LOG4CPP_SOURCE_DIR) install

$(LOG4CPP_SOURCE_DIR)/Makefile : $(LOG4CPP_SOURCE_DIR)/configure
	$(LOG4CPP_SOURCE_DIR)/configure --prefix=/usr/local

#raknet
$(RAKNET_LIB) : $(RAKNET_SOURCE_DIR)/Makefile
	$(MAKE) -C $(RAKNET_SOURCE_DIR)


objs/%.o: %.cpp
	$(CC) $(CFLAGS) -o $@ $<

clean :
	rm -f $(TARGET) objs/*.o objs/json/*.o
	

deps: $(DEPS_LIB)

clean_deps:
	$(MAKE) -C $(RAKNET_SOURCE_DIR) clean
	$(MAKE) -C $(EVENT_SOURCE_DIR) clean
	$(MAKE) -C $(LOG4CPP_SOURCE_DIR) clean
	$(MAKE) -C $(EVENT_SOURCE_DIR) uninstall
	$(MAKE) -C $(LOG4CPP_SOURCE_DIR) uninstall




$(TARGET) : $(TEMP) $(OBJS)
	$(AR) -x $(RAKNET_LIB)
	$(AR) -x $(EVENT_LIB)
	$(AR) -x $(EVENT_LIB_CORE)
	$(AR) -x $(EVENT_LIB_EXT)
	$(AR) -x $(LOG_LIB)

	$(AR) rcs $@ $(OBJS) *.o 
	rm -f *.o
	