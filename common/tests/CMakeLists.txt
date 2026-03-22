file(GLOB TEST_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")

if(TEST_SOURCES)
    add_executable(common_tests ${TEST_SOURCES})

    target_link_libraries(common_tests
            PRIVATE
            common
            gtest_main
            gmock_main
    )

    gtest_discover_tests(common_tests)
endif()
