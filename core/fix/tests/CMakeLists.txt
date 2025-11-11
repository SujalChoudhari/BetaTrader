cmake_minimum_required(VERSION 3.15)

file(GLOB_RECURSE ALL_TEST_SOURCES "src/*.cpp")
list(REMOVE_DUPLICATES ALL_TEST_SOURCES)

add_executable(fix_tests ${ALL_TEST_SOURCES})

target_link_libraries(fix_tests
        PUBLIC
        gtest
        gmock
        gtest_main
        fix_lib
)
