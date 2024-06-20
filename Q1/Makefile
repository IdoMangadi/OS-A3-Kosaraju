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
	./deque_AM < graph10000_300000.txt> deque_AM_output.txt 2>&1 || true
	gprof deque_AM gmon.out > deque_AM_analysis.txt
	./deque_AL < graph10000_300000.txt > deque_AL_output.txt 2>&1 || true
	gprof deque_AL gmon.out > deque_AL_analysis.txt
	./list_AL < graph10000_300000.txt > list_AL_output.txt 2>&1 || true
	gprof list_AL gmon.out > list_AL_analysis.txt
	./list_AM < graph10000_300000.txt > list_AM_output.txt 2>&1 || true
	gprof list_AM gmon.out > list_AM_analysis.txt

prof_analysis: run_prof
	gprof deque_AM gmon.out > deque_AM_analysis.txt
	gprof deque_AL gmon.out > deque_AL_analysis.txt
	gprof list_AL gmon.out > list_AL_analysis.txt
	gprof list_AM gmon.out > list_AM_analysis.txt

clean:
	rm -f *.o deque_AM deque_AL list_AL list_AM

cleanAll: clean
	rm -f *.gcda *.gcno *.gcov gmon.out 

# git:
cps:
	git commit -a -m "$(m)"
	git push
	git status