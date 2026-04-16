cmake_minimum_required(VERSION 3.15)
project(client_admin LANGUAGES CXX)

file(GLOB_RECURSE ADMIN_SOURCES "src/*.cpp")

add_library(client_admin_lib STATIC ${ADMIN_SOURCES})

target_include_directories(client_admin_lib PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/exchange/exchange_app/include
    ${CMAKE_SOURCE_DIR}/exchange/exchange_fix/include
    ${CMAKE_SOURCE_DIR}/vendor/lib/imgui
    ${CMAKE_SOURCE_DIR}/vendor/lib/asio/include
    ${CMAKE_SOURCE_DIR}/vendor/lib/SPSCQueue/include
)

target_link_libraries(client_admin_lib
    PUBLIC
    common
    exchange_app_lib
    fix_lib
    client_ui_lib
)

