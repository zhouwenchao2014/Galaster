UNAME := $(shell uname)
.PHONY: run test clean

vpath %.hh src
vpath %.cc src

ifeq ($(UNAME), Darwin)
libgalaster.so: galaster.cc geometry.cc bezier.cc vec3d.cc prop.cc
	g++ -o $@ $^ -std=c++11 --shared -fPIC -I ./glfw -L ./glfw -lglfw3 \
		-framework Cocoa -framework OpenGL -framework IOKit -framework CoreVideo \
		-Wall -Wextra -Wno-deprecated-declarations -pedantic -g -O3 -march=native
endif
ifeq ($(UNAME), Linux)
	g++ -o $@ $^ -std=c++11 --shared -fPIC -I ./glfw -L ./glfw -lglfw \
		-lGL -lGLU \
		-Wall -Wextra -Wno-deprecated-declarations -pedantic -g -O3 -march=native
endif

ifeq ($(UNAME), Darwin)
CXXFLAGS += -framework OpenGL
endif
ifeq ($(UNAME), Linux)
CXXFLAGS += -lGL -lGLU
endif

# test: test.cc
# 	g++ -o $@ $^ -std=c++11 -Wall -Wextra -pedantic -g

clean:
	rm -f libgalaster.so *.dot *.pdf
