CC = g++
CFLAGS = -Wall -g -pg
SRC = $(wildcard *.cpp)

.PHONY: all clean

all: deque_AL



deque_AL: deque_AL.o
	$(CC) $(CFLAGS) -o $@ $^

%.o: %.cpp
	$(CC) $(CFLAGS) -c $<

# profiling:
run_prof: deque_AL
	./deque_AL < graph10000_300000.txt > deque_AL_output.txt 2>&1 || true


clean:
	rm -f *.o deque_AL

cleanAll: clean
	rm -f *.gcda *.gcno *.gcov gmon.out 

# git:
cps:
	git commit -a -m "$(m)"
	git push
	git status