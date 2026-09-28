add_executable(client_ohlc_tests
    CandleAggregatorTests.cpp
)

target_link_libraries(client_ohlc_tests
    PRIVATE
    client_ohlc_lib
    data_lib
    common
    gtest_main
)

gtest_discover_tests(client_ohlc_tests)
