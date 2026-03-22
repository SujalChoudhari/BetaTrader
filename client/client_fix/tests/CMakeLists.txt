cmake_minimum_required(VERSION 3.15)

# Assuming gtest/gmock is available from the parent CMake (via vendor directory in project root)

add_executable(client_fix_tests 
    src/ClientFixEndToEndTests.cpp
    src/SeqNumStoreTests.cpp
    src/FixMessageParserTests.cpp
    src/AuthManagerTests.cpp
    src/FixClientSessionTests.cpp
)

target_include_directories(client_fix_tests PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/../include
)

target_link_libraries(client_fix_tests
        PUBLIC
        gtest
        gmock
        gtest_main
        client_fix_lib
        fix_lib           # To stand up the server
        trading_core_lib  # Server depends on core
        common
)

include(GoogleTest)
gtest_discover_tests(client_fix_tests)
