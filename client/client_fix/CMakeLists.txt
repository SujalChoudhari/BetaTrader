cmake_minimum_required(VERSION 3.15)
project(client_fix LANGUAGES CXX)

find_package(Threads REQUIRED)

file(GLOB_RECURSE CLIENT_FIX_SOURCES "src/*.cpp")

add_library(client_fix_lib ${CLIENT_FIX_SOURCES})

target_include_directories(client_fix_lib PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${CMAKE_SOURCE_DIR}/vendor/lib/asio/include
        ${CMAKE_SOURCE_DIR}/vendor/lib/SPSCQueue/include
        ${CMAKE_SOURCE_DIR}/vendor/lib/imgui
)

target_link_libraries(client_fix_lib
        PUBLIC
        common
        Threads::Threads
)