cmake_minimum_required(VERSION 2.8)
project(Galaster)

find_package(OpenGL)

# GLFW's cmake module is broken (glfw #483), use pkg-config now.
find_package(PkgConfig)
pkg_check_modules(GLFW REQUIRED glfw3)

add_definitions(-std=c++11 -Wall -O3 -march=native -g)

add_subdirectory(src)
add_subdirectory(examples)
