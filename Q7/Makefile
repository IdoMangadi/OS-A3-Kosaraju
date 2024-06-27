CC = g++ 
CFLAGS = -Wall -g -std=c++11 -pthread
CFLAGS_C = -Wall -g
SRC_CPP = $(wildcard *.cpp)
SRC_C = $(wildcard *.c)
OBJ_CPP = $(SRC_CPP:.cpp=.o)
OBJ_C = $(SRC_C:.c=.o)

.PHONY: all clean

all: threadsServer 

threadsServer: threadsServer.o deque_AL.o
	$(CC) $(CFLAGS) $^ -o $@

%.o: %.cpp
	$(CC) $(CFLAGS) -c $<

%.o: %.c
	$(CC) $(CFLAGS_C) -c $<

clean:
	rm -f *.o threadsServer

cleanAll: clean
	rm -f *.gcda *.gcno *.gcov gmon.out