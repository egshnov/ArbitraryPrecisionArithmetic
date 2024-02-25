project(Test)

set(test_source main.c)
set(HEADERS minunit.h)
add_executable(tst ${test_source} ${HEADERS})

target_link_libraries(tst ArbitaryPrecisionArithmetics)

add_test(NAME Test1 COMMAND tst)
