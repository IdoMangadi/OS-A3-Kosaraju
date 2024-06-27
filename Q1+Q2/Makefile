CC = g++
CFLAGS = -Wall -g -std=c++11 -pg
SRC = $(wildcard *.cpp)

.PHONY: all clean

all: deque_AM deque_AL list_AL list_AM

deque_AM: deque_AM.o
	$(CC) $(CFLAGS) -o $@ $^

deque_AL: deque_AL.o
	$(CC) $(CFLAGS) -o $@ $^

list_AL: list_AL.o
	$(CC) $(CFLAGS) -o $@ $^

list_AM: list_AM.o
	$(CC) $(CFLAGS) -o $@ $^

%.o: %.cpp
	$(CC) $(CFLAGS) -c $<

# profiling:
run_prof: deque_AM deque_AL list_AL list_AM
	echo "----Profiling deque_AM, deque_AL, list_AL, list_AM with graph10000_300000.txt----"
	./deque_AM < inputs/graph10000_300000.txt> /dev/null || true
	gprof deque_AM gmon.out > prof_reports/deque_AM_analysis.txt
	./deque_AL < inputs/graph10000_300000.txt > /dev/null || true
	gprof deque_AL gmon.out > prof_reports/deque_AL_analysis.txt
	./list_AL < inputs/graph10000_300000.txt > /dev/null || true
	gprof list_AL gmon.out > prof_reports/list_AL_analysis.txt
	./list_AM < inputs/graph10000_300000.txt > /dev/null || true
	gprof list_AM gmon.out > prof_reports/list_AM_analysis.txt

clean:
	rm -f *.o deque_AM deque_AL list_AL list_AM

cleanAll: clean
	rm -f *.gcda *.gcno *.gcov gmon.out 
	