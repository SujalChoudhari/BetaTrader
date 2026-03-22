file(GLOB TEST_SOURCES "*.cpp")

if(TEST_SOURCES)
    add_executable(exchange_persistence_tests ${TEST_SOURCES})
    target_link_libraries(exchange_persistence_tests
        PRIVATE
        data_lib
        common
        gtest_main
        gmock
    )
    gtest_discover_tests(exchange_persistence_tests)
endif()