CC = g++
CFLAGS = -Wall -g -std=c++11
SRC = $(wildcard *.cpp)
OBJ = $(SRC:.cpp=.o)

.PHONY: all clean

all: deque_AL

deque_AL: $(OBJ)
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
