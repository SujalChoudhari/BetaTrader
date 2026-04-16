cmake_minimum_required(VERSION 3.15)
project(client_ohlc LANGUAGES CXX)

file(GLOB_RECURSE OHLC_SOURCES "src/*.cpp")

add_library(client_ohlc_lib STATIC ${OHLC_SOURCES})

target_include_directories(client_ohlc_lib PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/common/include
    ${CMAKE_SOURCE_DIR}/exchange/exchange_persistence/include
    ${CMAKE_SOURCE_DIR}/vendor/lib/SQLiteCpp/include
    ${CMAKE_SOURCE_DIR}/vendor/lib/SPSCQueue/include
)

target_link_libraries(client_ohlc_lib
    PUBLIC
    common
    data_lib
)
