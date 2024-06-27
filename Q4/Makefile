CC = g++ 
CFLAGS = -Wall -g -std=c++11
CFLAGS_C = -Wall -g
SRC_CPP = $(wildcard *.cpp)
SRC_C = $(wildcard *.c)
OBJ_CPP = $(SRC_CPP:.cpp=.o)
OBJ_C = $(SRC_C:.c=.o)

.PHONY: all clean

all: chatLike 

chatLike: $(OBJ_CPP) 
	$(CC) $(CFLAGS) -o $@ $^

%.o: %.cpp
	$(CC) $(CFLAGS) -c $<

%.o: %.c
	$(CC) $(CFLAGS_C) -c $<

# profiling:
run_prof: chatLike
	./chatLike < ../Q1+Q2/inputs/graph10000_300000.txt > ../Q1+Q2/prof_reports/deque_AL_output.txt 2>&1 || true

clean:
	rm -f *.o chatLike 

cleanAll: clean
	rm -f *.gcda *.gcno *.gcov gmon.out

# git:
cps:
	git commit -a -m "$(m)"
	git push
	git status
