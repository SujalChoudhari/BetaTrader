file(GLOB TEST_SOURCES "src/*.cpp")

if(TEST_SOURCES)
    add_executable(exchange_fix_tests ${TEST_SOURCES})
    target_link_libraries(exchange_fix_tests
        PRIVATE
        fix_lib
        common
        gtest_main
        gmock
    )
    gtest_discover_tests(exchange_fix_tests)
endif()
