UNAME := $(shell uname)
.PHONY: run test clean
run: main.cc geometry.cc bezier.cc
ifeq ($(UNAME), Darwin)
	g++ -o $@ $^ -std=c++11 -I ./glfw -L ./glfw -lglfw3 -framework Cocoa -framework OpenGL -framework IOKit -framework CoreVideo  -Wall -Wextra -Wno-deprecated-declarations -pedantic -g -O3 -march=native
endif
ifeq ($(UNAME), Linux)
	g++ -o $@ $^ -std=c++11 -I ./glfw -L ./glfw -lglfw -lGL -lGLU -Wall -Wextra -Wno-deprecated-declarations -pedantic -g -O3 -march=native
endif

test: test.cc
	g++ -o $@ $^ -std=c++11 -Wall -Wextra -pedantic -g

clean:
	rm -f run test *.dot *.pdf
