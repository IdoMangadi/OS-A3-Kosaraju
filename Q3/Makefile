CC = g++
CFLAGS = -Wall -g -std=c++11
SRC = $(wildcard *.cpp)
OBJ = $(SRC:.cpp=.o)

.PHONY: all clean

all: kosaraju

kosaraju: $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^

%.o: %.cpp
	$(CC) $(CFLAGS) -c $<

# profiling:
run_prof: kosaraju
	./kosaraju < graph10000_300000.txt > deque_AL_output.txt 2>&1 || true

clean:
	rm -f *.o kosaraju

cleanAll: clean
	rm -f *.gcda *.gcno *.gcov gmon.out

# git:
cps:
	git commit -a -m "$(m)"
	git push
	git status
