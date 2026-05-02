cmake_minimum_required(VERSION 3.15)

find_package(Threads REQUIRED)
file(GLOB_RECURSE ALL_FIX_SOURCES "src/*.cpp")
list(REMOVE_ITEM ALL_FIX_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/src/Main.cpp")
list(REMOVE_DUPLICATES ALL_FIX_SOURCES)

add_library(fix_lib ${ALL_FIX_SOURCES})

target_include_directories(fix_lib PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${CMAKE_SOURCE_DIR}/vendor/lib/asio/include
)

target_link_libraries(fix_lib
        PUBLIC
        common
        exchange_app_lib
        Threads::Threads
)
target_compile_definitions(fix_lib PRIVATE _WIN32_WINNT=0x0601)
target_compile_definitions(fix_lib PUBLIC ASIO_STANDALONE)

add_executable(exchange_app src/Main.cpp)

target_link_libraries(exchange_app
        PUBLIC
        fix_lib
        common
        Threads::Threads
)
target_compile_definitions(exchange_app PRIVATE _WIN32_WINNT=0x0601)
