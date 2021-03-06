cmake_minimum_required(VERSION 2.8)
project(Galaster)

if (APPLE)
   SET(CMAKE_C_COMPILER "/usr/local/bin/clang-omp")
   SET(CMAKE_CXX_COMPILER "/usr/local/bin/clang-omp++")
else ()
   find_package(OpenMP)
   set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
endif ()

find_package(OpenGL)

# GLFW's cmake module is broken (glfw #483), use pkg-config now.
find_package(PkgConfig)
pkg_check_modules(GLFW REQUIRED glfw3)

# FreeType 2
pkg_check_modules(FREETYPE2 REQUIRED freetype2)

add_definitions(-std=c++11 -Wall -Ofast -march=native -g -fopenmp)

add_subdirectory(src)
add_subdirectory(examples)
