add_executable(OrderBookManagerTests src/OrderBookManagerTests.cpp)
target_link_libraries(OrderBookManagerTests
    gtest_main
    client_orderbook_lib
)
gtest_discover_tests(OrderBookManagerTests)
