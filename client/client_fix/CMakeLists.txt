cmake_minimum_required(VERSION 3.15)
project(client_fix LANGUAGES CXX)

find_package(Threads REQUIRED)

file(GLOB_RECURSE CLIENT_FIX_SOURCES "src/*.cpp")
# Main is generally ignored if it's an app, but here it's a lib plus potential tests.

add_library(client_fix_lib ${CLIENT_FIX_SOURCES})

target_include_directories(client_fix_lib PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${CMAKE_SOURCE_DIR}/vendor/lib/asio/include
        ${CMAKE_SOURCE_DIR}/vendor/lib/SPSCQueue/include
)

target_link_libraries(client_fix_lib
        PUBLIC
        common
        Threads::Threads
)
# ASIO requires this definition on Windows sometimes, harmless on Linux
target_compile_definitions(client_fix_lib PRIVATE _WIN32_WINNT=0x0601)
target_compile_definitions(client_fix_lib PUBLIC ASIO_STANDALONE)

# Tests Subdirectory
if(BUILD_TESTING OR ENABLE_COVERAGE OR ENABLE_TESTING)
    add_subdirectory(tests)
endif()
