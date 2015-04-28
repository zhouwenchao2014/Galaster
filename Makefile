.PHONY: run
run: main.cc
	g++ -o $@ $^ -std=c++11 -Wall -Wextra -pedantic -g
