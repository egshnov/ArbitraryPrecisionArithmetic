cmake_minimum_required(VERSION 3.25)
project(ArbPrecHW C)

set(CMAKE_C_STANDARD 17)

enable_testing()

include_directories(lib)

add_subdirectory(tests)
add_subdirectory(lib)

add_executable(ArbPrecHW main.c)
target_link_libraries(ArbPrecHW ArbitaryPrecisionArithmetics)