CC = g++ 
CFLAGS = -Wall -g -std=c++11 -pthread
CFLAGS_C = -Wall -g
VALGRIND_FLAGS = -v --leak-check=full --show-leak-kinds=all  --error-exitcode=99
SRC_CPP = $(wildcard *.cpp)
SRC_C = $(wildcard *.c)
OBJ_CPP = $(SRC_CPP:.cpp=.o)
OBJ_C = $(SRC_C:.c=.o)

.PHONY: all clean 

all: libpatterns proactorServer 

libpatterns: patterns.o
	ar rcs libpatterns.a patterns.o

proactorServer: deque_AL.o proactorServer.o libpatterns
	$(CC) $(CFLAGS) deque_AL.o proactorServer.o -L. -lpatterns -o $@

%.o: %.cpp
	$(CC) $(CFLAGS) -c $<

%.o: %.c
	$(CC) $(CFLAGS_C) -c $<

valgrind: proactorServer
	valgrind --tool=memcheck $(VALGRIND_FLAGS) ./proactorServer | { egrep "lost| at " || true; }

clean:
	rm -f *.o *.a proactorServer  

cleanAll: clean
	rm -f *.gcda *.gcno *.gcov gmon.out
	