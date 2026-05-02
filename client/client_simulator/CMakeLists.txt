cmake_minimum_required(VERSION 3.15)

add_executable(client_simulator src/Main.cpp)

target_include_directories(client_simulator PRIVATE include)

target_link_libraries(client_simulator
    PRIVATE
    client_fix_lib
    common
    spdlog::spdlog
)
