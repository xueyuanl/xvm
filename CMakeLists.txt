cmake_minimum_required(VERSION 3.13)
project(xvm C)

set(CMAKE_C_STANDARD 99)
add_executable(xvm src/main.c src/structure.c src/structure.h src/constants.h)
