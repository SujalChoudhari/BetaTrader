cmake_minimum_required(VERSION 4.0)
project(BetaTrader LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_subdirectory(common)
add_subdirectory(core/trading_core)
add_subdirectory(core/data)

enable_testing()

set(gtest_build_tests OFF CACHE BOOL "")

add_subdirectory(vendor/lib/googletest)
add_subdirectory(vendor/lib/SQLiteCpp)
add_subdirectory(vendor/lib/spdlog)

add_subdirectory(vendor/lib/SPSCQueue)
include_directories(vendor/lib/SPSCQueue/include)

add_subdirectory(core/trading_core/tests)
add_subdirectory(core/data/tests)
