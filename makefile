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

lib.o: lib.cpp
lib.cpp: lib.h
main.o: main.cpp

%.o: %.cpp
	$(CC) $(CFLAGS) $< -o $@

$(TARGET): $(OBJS)
	$(CC) $(LFLAGS) $^ -o $@
	./$(TARGET) > results.txt

clean:
	rm *.o
	rm $(TARGET)
