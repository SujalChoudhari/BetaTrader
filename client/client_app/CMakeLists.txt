cmake_minimum_required(VERSION 3.15)
project(client_app LANGUAGES CXX)

add_executable(client_app 
    src/Main.cpp
    src/App.cpp
)


target_link_libraries(client_app
    PRIVATE
    client_ui_lib
    client_fix_lib
    client_admin_lib
    client_orderbook_lib
    client_ohlc_lib
    client_simulator_lib
    data_lib
    common
    spdlog
)


# Required for thread support
find_package(Threads REQUIRED)
target_link_libraries(client_app PRIVATE Threads::Threads)
