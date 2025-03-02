CC=gcc
CFLAGS=-c -std=c++11
LFLAGS=-lstdc++
TARGET=bt_sitzvert

#SRCS=$(wildcard *.cpp)
SRCS=bundestag_lib.cpp main.cpp
OBJS=$(patsubst %.cpp,%.o,$(SRCS))

all: $(TARGET)

debug: CFLAGS += -g -Wall
debug: $(TARGET)

%.o: %.cpp
	$(CC) $(CFLAGS) $< -o $@

$(TARGET): $(OBJS)
	$(CC) $(LFLAGS) $^ -o $@

clean:
	rm *.o
	rm $(TARGET)
