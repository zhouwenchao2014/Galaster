CXXFLAGS += -std=c++11 -I ../src -I ../glfw -L ../ -lgalaster -O3 -march=native

UNAME := $(shell uname)
ifeq ($(UNAME), Darwin)
CXXFLAGS += -framework OpenGL
endif
ifeq ($(UNAME), Linux)
CXXFLAGS += -lGL -lGLU
endif

targets = libgalaster.so binary_tree cube membrane random randspline multiedge splineorama

.PHONY: all clean
all: $(targets)
libgalaster.so:
	make -C ../ && ln -s ../$@ ./$@

clean:
	rm -f $(targets) 
