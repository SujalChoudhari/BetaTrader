cmake_minimum_required(VERSION 3.15)
project(client_app LANGUAGES CXX)

add_library(client_app_lib STATIC
    src/App.cpp
)

target_include_directories(client_app_lib PUBLIC include src)

target_link_libraries(client_app_lib
    PUBLIC
    client_ui_lib
    client_fix_lib
    client_admin_lib
    common
    spdlog
)

add_executable(client_app src/Main.cpp)
target_link_libraries(client_app PRIVATE client_app_lib)

# Required for thread support
find_package(Threads REQUIRED)
target_link_libraries(client_app_lib PUBLIC Threads::Threads)
target_link_libraries(client_app PRIVATE Threads::Threads)
