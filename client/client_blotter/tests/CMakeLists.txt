add_executable(client_blotter_tests
    BlotterModelTests.cpp
)

target_link_libraries(client_blotter_tests
    PRIVATE
    client_blotter_lib
    common
    gtest_main
)

gtest_discover_tests(client_blotter_tests)
