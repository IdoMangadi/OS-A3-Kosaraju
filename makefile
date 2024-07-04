FLAGS = -Wall -g

.PHONY: all clean Q1Q2 Q3 Q4 Q5Q6 Q7 Q8Q9Q10 cps

all: Q1Q2 Q3 Q4 Q5Q6 Q7 Q8Q9Q10

Q1Q2:
	make -C Q1+Q2 all

Q3:
	make -C Q3 all

Q4:
	make -C Q4 all

Q5Q6: 
	make -C Q5+Q6 all

Q7:
	make -C Q7 all

Q8Q9Q10:
	make -C Q8+Q9+Q10 all

# Cleaning:
clean:
	make -C Q1+Q2 clean
	make -C Q3 clean
	make -C Q4 clean
	make -C Q5+Q6 clean
	make -C Q7 clean
	make -C Q8+Q9+Q10 clean

# git:
cps:
	git commit -a -m "$(m)"
	git push
	git status
	