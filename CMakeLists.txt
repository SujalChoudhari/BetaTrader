cmake_minimum_required(VERSION 3.15)
project(BetaTrader LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Ensure all binaries go to a single bin folder for easy local testing
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

option(ENABLE_COVERAGE "Enable code coverage generation" OFF)

if(ENABLE_COVERAGE)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} --coverage -fprofile-arcs -ftest-coverage")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} --coverage")
    set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} --coverage")

    find_program(LCOV_PATH lcov REQUIRED)
    find_program(GENHTML_PATH genhtml REQUIRED)

    add_custom_target(coverage
        # Reset counters
        COMMAND ${LCOV_PATH} --directory ${CMAKE_BINARY_DIR} --zerocounters
            --ignore-errors empty
        # Run tests
        COMMAND ${CMAKE_CTEST_COMMAND} --test-dir ${CMAKE_BINARY_DIR} --output-on-failure
        # Capture coverage data
        COMMAND ${LCOV_PATH} --directory ${CMAKE_BINARY_DIR} --capture
            --output-file ${CMAKE_BINARY_DIR}/coverage_raw.info
            --ignore-errors mismatch,empty,negative,gcov,inconsistent
        # Filter out vendor, test, and system files
        COMMAND ${LCOV_PATH}
            --remove ${CMAKE_BINARY_DIR}/coverage_raw.info
            '*/vendor/*' '*/tests/*' '*/test/*' '/usr/*' '*/googletest/*' '*/build/*'
            --output-file ${CMAKE_BINARY_DIR}/coverage.info
            --ignore-errors unused,negative
        # Generate HTML report (optional but useful)
        COMMAND ${GENHTML_PATH} ${CMAKE_BINARY_DIR}/coverage.info
            --output-directory ${CMAKE_BINARY_DIR}/coverage_html
            --ignore-errors unmapped
        COMMAND ${CMAKE_COMMAND} -E echo "Coverage info: ${CMAKE_BINARY_DIR}/coverage.info"
        COMMAND ${CMAKE_COMMAND} -E echo "HTML report:   ${CMAKE_BINARY_DIR}/coverage_html/index.html"
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
        COMMENT "Generating code coverage report..."
        VERBATIM
    )
endif()

if(ENABLE_COVERAGE)
    include(CTest)  # Creates DartConfiguration.tcl needed by "ctest -T Coverage"
else()
    enable_testing()
endif()

include(GoogleTest)
set(gtest_build_tests OFF CACHE BOOL "")


add_subdirectory(vendor/lib/googletest)
add_subdirectory(vendor/lib/SQLiteCpp)
add_subdirectory(vendor/lib/spdlog)
add_subdirectory(vendor/lib/SPSCQueue)

# GLFW for Client UI
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "")
set(GLFW_BUILD_TESTS OFF CACHE BOOL "")
set(GLFW_BUILD_DOCS OFF CACHE BOOL "")
set(GLFW_INSTALL OFF CACHE BOOL "")
set(GLFW_BUILD_WAYLAND OFF CACHE BOOL "")
add_subdirectory(vendor/lib/glfw)


include_directories(vendor/lib/SPSCQueue/include)
include_directories(vendor/lib/asio/include)

add_subdirectory(common)
add_subdirectory(common/tests)
add_subdirectory(client/client_fix)
add_subdirectory(client/client_orderbook)
add_subdirectory(client/client_ui)
add_subdirectory(client/client_admin)
add_subdirectory(client/client_app)
add_subdirectory(client/client_simulator)

add_subdirectory(client/client_fix/tests)
add_subdirectory(client/client_orderbook/tests)
add_subdirectory(client/client_app/tests)
add_subdirectory(exchange)

add_subdirectory(exchange/exchange_state/tests)
add_subdirectory(exchange/exchange_matching/tests)
add_subdirectory(exchange/exchange_risk/tests)
add_subdirectory(exchange/exchange_app/tests)
add_subdirectory(exchange/exchange_fix/tests)
add_subdirectory(exchange/exchange_persistence/tests)
