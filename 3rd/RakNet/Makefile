CC = g++
AR = ar
CFLAGS = -c -fPIC -I.
CFLAGS += -Wall -Werror -g -O2
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