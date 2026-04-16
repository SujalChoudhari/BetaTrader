cmake_minimum_required(VERSION 3.15)
project(client_orderbook LANGUAGES CXX)

file(GLOB_RECURSE ORDERBOOK_SOURCES "src/*.cpp")

add_library(client_orderbook_lib STATIC ${ORDERBOOK_SOURCES})

target_include_directories(client_orderbook_lib PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/common/include
)

target_link_libraries(client_orderbook_lib
    PUBLIC
    common
)
