CC = g++
AR = ar

EVENT_SOURCE_DIR = ../../3rd/libevent
LOG4CPP_SOURCE_DIR = ../../3rd/log4cpp
RAKNET_SOURCE_DIR = ../../3rd/RakNet

CFLAGS = -c -fPIC
CFLAGS += -I.
CFLAGS += -I./$(RAKNET_SOURCE_DIR)/Source

ifdef RELEASE
	CFLAGS += -Wall -Werror -g -O2
else
	CFLAGS += -Wall -Werror -g -D_DEBUG
endif


EVENT_LIB = /usr/local/lib/libevent.a 
EVENT_LIB_CORE = /usr/local/lib/libevent_core.a 
EVENT_LIB_EXT = /usr/local/lib/libevent_extra.a 
LOG_LIB = /usr/local/lib/liblog4cpp.a
RAKNET_LIB = ../../3rd/RakNet/Lib/libraknet.a

TARGET = ../lib/libcommon.a
SOURCES = $(wildcard *.cpp)
OBJS    = $(RAKNET_LIB) $(patsubst %.cpp,objs/%.o,$(SOURCES)) 


default : $(TARGET)

#libevent
$(EVENT_LIB) : $(EVENT_SOURCE_DIR)/Makefile
	$(MAKE) -C $(EVENT_SOURCE_DIR)
	$(MAKE) -C $(EVENT_SOURCE_DIR) install

$(EVENT_SOURCE_DIR)/Makefile : $(EVENT_SOURCE_DIR)/configure
	./$(EVENT_SOURCE_DIR)/configure --prefix=/usr/local

#log4cpp
$(LOG_LIB) : $(LOG4CPP_SOURCE_DIR)/Makefile
	$(MAKE) -C $(LOG4CPP_SOURCE_DIR)
	$(MAKE) -C $(LOG4CPP_SOURCE_DIR) install

$(LOG4CPP_SOURCE_DIR)/Makefile : $(LOG4CPP_SOURCE_DIR)/configure
	./$(LOG4CPP_SOURCE_DIR)/configure --prefix=/usr/local

#raknet
$(RAKNET_LIB) : $(RAKNET_SOURCE_DIR)/Makefile
	$(MAKE) -C $(RAKNET_SOURCE_DIR)


objs/%.o: %.cpp
	$(CC) $(CFLAGS) -o $@ $<

clean :
	rm -f $(TARGET) objs/*.o

deps: $(EVENT_SOURCE_DIR)/configure $(LOG4CPP_SOURCE_DIR)/configure
	$(MAKE) -C $(LOG4CPP_SOURCE_DIR)
	$(MAKE) -C $(LOG4CPP_SOURCE_DIR) install
	$(MAKE) -C $(EVENT_SOURCE_DIR)
	$(MAKE) -C $(EVENT_SOURCE_DIR) install

$(TARGET) : $(TEMP) $(OBJS)
	$(AR) -x $(RAKNET_LIB)
	$(AR) -x $(EVENT_LIB)
	$(AR) -x $(EVENT_LIB_CORE)
	$(AR) -x $(EVENT_LIB_EXT)
	$(AR) -x $(LOG_LIB)

	$(AR) rcs $@ $(OBJS) *.o
	rm -f *.o
	