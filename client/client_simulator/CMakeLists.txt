cmake_minimum_required(VERSION 3.15)
project(client_simulator LANGUAGES CXX)

file(GLOB_RECURSE SIM_SOURCES "src/*.cpp")

add_library(client_simulator_lib STATIC ${SIM_SOURCES})

target_include_directories(client_simulator_lib PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/common/include
    ${CMAKE_SOURCE_DIR}/exchange/exchange_app/include
    ${CMAKE_SOURCE_DIR}/exchange/exchange_routing/include
    ${CMAKE_SOURCE_DIR}/exchange/exchange_matching/include
    ${CMAKE_SOURCE_DIR}/exchange/exchange_risk/include
    ${CMAKE_SOURCE_DIR}/exchange/exchange_state/include
    ${CMAKE_SOURCE_DIR}/exchange/exchange_persistence/include
    ${CMAKE_SOURCE_DIR}/exchange/exchange_publishers/include
    ${CMAKE_SOURCE_DIR}/vendor/lib/asio/include
    ${CMAKE_SOURCE_DIR}/vendor/lib/SPSCQueue/include
)

target_link_libraries(client_simulator_lib
    PUBLIC
    common
    exchange_app_lib
)
