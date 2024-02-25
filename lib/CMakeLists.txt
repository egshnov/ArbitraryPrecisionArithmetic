cmake_minimum_required(VERSION 3.25)
project(ArbitaryPrecisionArithmetics C)

set(CMAKE_C_STANDARD 17)
set(SOURCES number.c)
set(HEADERS number.h)
include_directories(lib)
add_library(ArbitaryPrecisionArithmetics STATIC ${HEADERS} ${SOURCES})