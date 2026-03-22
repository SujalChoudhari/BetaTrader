cmake_minimum_required(VERSION 3.15)

add_executable(fix_tests 
    src/BinaryToOrderRequestConverterTests.cpp
    src/ExecutionReportToBinaryConverterTests.cpp
    src/FixServerSessionTests.cpp
    src/FixSessionManagerTests.cpp
    src/FixUtilsTests.cpp
    src/FixEndToEndTests.cpp
)

# Link against required libraries
target_link_libraries(fix_tests
        PUBLIC
        gtest
        gmock
        gtest_main
        fix_lib
        trading_core_lib # The mock core needs this
)

include(GoogleTest)
gtest_discover_tests(fix_tests)
