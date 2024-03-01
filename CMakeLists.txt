cmake_minimum_required(VERSION 3.25)
project(ArbPrecHW C)

set(CMAKE_C_STANDARD 17)

enable_testing()

include_directories(lib) # for main.c

add_subdirectory(tests)
add_subdirectory(lib)