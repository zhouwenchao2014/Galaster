.PHONY: run clean
run: main.cc
	g++ -o $@ $^ -std=c++11 -Wall -Wextra -pedantic -g

clean:
	rm -f run *.dot *.pdf
