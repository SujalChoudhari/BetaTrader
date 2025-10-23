add_executable(trading_core_tests
        OrderManagerTests.cpp
        OrderBookTests.cpp

)

target_link_libraries(trading_core_tests
        PRIVATE
        trading_core_lib
        common
        gtest_main
)

include(GoogleTest)
gtest_add_tests(TARGET trading_core_tests)