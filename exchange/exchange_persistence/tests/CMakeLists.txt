file(GLOB TEST_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")


add_executable(data_tests
        ${TEST_SOURCES}
)


target_link_libraries(data_tests
        PRIVATE
        data_lib
        common
        gtest_main
        SQLiteCpp sqlite3
)

include(GoogleTest)
gtest_discover_tests(data_tests)