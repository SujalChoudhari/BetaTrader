file(GLOB TEST_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")


add_executable(trading_core_tests
        ${TEST_SOURCES}
)


target_link_libraries(trading_core_tests
        PRIVATE
        trading_core_lib
        common
        gtest_main
)

include(GoogleTest)
gtest_add_tests(TARGET trading_core_tests)