cmake_minimum_required(VERSION 3.15)
project(client_blotter LANGUAGES CXX)

add_library(client_blotter_lib STATIC
    src/BlotterModel.cpp
)

target_include_directories(client_blotter_lib PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/common/include
)

target_link_libraries(client_blotter_lib PUBLIC common)
