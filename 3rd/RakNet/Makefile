CC = g++
AR = ar
CFLAGS = -c -fPIC -I.
ifdef RELEASE
	CFLAGS += -Wall -Werror -g -O2
else
	CFLAGS += -Wall -Werror -g
endif
TARGET = lib/libraknet.a
SOURCES = $(wildcard Source/*.cpp)
OBJS    = $(patsubst Source/%.cpp,Source/%.o,$(SOURCES))

default : $(TARGET)

Source/%.o: %.cpp
	$(CC) $(CFLAGS) -o $@ $<

clean :
	rm -f $(TARGET) Source/*.o

$(TARGET) : $(OBJS)
	$(AR) rcs $@ $(OBJS)