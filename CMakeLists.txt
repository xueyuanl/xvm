cmake_minimum_required(VERSION 3.13)
project(xvm C)

set(CMAKE_C_STANDARD 99)
add_executable(xvm src/main.c
        src/structure.c
        src/structure.h
        src/constants.h
        src/instruction.c
        src/instruction.h
        src/global.c
        src/global.h
        src/stack.c
        src/stack.h
        src/function.c
        src/function.h
        src/host_api.c
        src/host_api.h)
target_link_libraries(xvm m)