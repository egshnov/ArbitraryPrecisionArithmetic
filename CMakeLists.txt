cmake_minimum_required(VERSION 3.10)
project(ArbPrecHW C)

set(CMAKE_C_STANDARD 17)

enable_testing()

add_subdirectory(tests)
add_subdirectory(lib)

target_include_directories(tst PUBLIC lib)