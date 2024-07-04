CC = g++ 
CFLAGS = -Wall -g -std=c++11 -pthread
CFLAGS_C = -Wall -g
VALGRIND_FLAGS = -v --leak-check=full --show-leak-kinds=all  --error-exitcode=99
SRC_CPP = $(wildcard *.cpp)
SRC_C = $(wildcard *.c)
OBJ_CPP = $(SRC_CPP:.cpp=.o)
OBJ_C = $(SRC_C:.c=.o)

.PHONY: all clean 

all: libpatterns reactorServer client 

libpatterns: patterns.o
	ar rcs libpatterns.a patterns.o

reactorServer: deque_AL.o reactorServer.o libpatterns
	$(CC) $(CFLAGS) deque_AL.o reactorServer.o -L. -lpatterns -o $@

client: client.cpp
	$(CC) $(CFLAGS) $^ -o $@

%.o: %.cpp
	$(CC) $(CFLAGS) -c $<

%.o: %.c
	$(CC) $(CFLAGS_C) -c $<

valgrind: reactorServer
	valgrind --tool=memcheck $(VALGRIND_FLAGS) ./reactorServer | { egrep "lost| at " || true; }

clean:
	rm -f *.o *.a reactorServer client 

cleanAll: clean
	rm -f *.gcda *.gcno *.gcov gmon.out
	