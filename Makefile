.PHONY: run test clean
run: main.cc
	g++ -o $@ $^ -std=c++11 -I ./glfw -L ./glfw -lglfw3 -framework Cocoa -framework OpenGL -framework IOKit -framework CoreVideo  -Wall -Wextra -Wno-deprecated-declarations -pedantic -g -O3 -march=native

test: test.cc
	g++ -o $@ $^ -std=c++11 -Wall -Wextra -pedantic -g

clean:
	rm -f run test *.dot *.pdf
