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
#	save results to text file
	./$(TARGET) kerg_mod.csv 3 > results_before2020.txt
	./$(TARGET) kerg_mod.csv 2 > results_reform2020.txt
	./$(TARGET) kerg_mod.csv 1 > results_reform2024.txt

clean:
	rm *.o
	rm $(TARGET)
