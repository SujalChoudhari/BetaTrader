cmake_minimum_required(VERSION 3.15)

file(GLOB_RECURSE ALL_CORE_SOURCES "src/*.cpp")
list(FILTER ALL_CORE_SOURCES EXCLUDE REGEX "[/\\]Main\\.cpp$")
list(REMOVE_DUPLICATES ALL_CORE_SOURCES)

add_library(trading_core_lib ${ALL_CORE_SOURCES})

target_include_directories(trading_core_lib PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${CMAKE_SOURCE_DIR}/vendor/lib/SPSCQueue/include
)

target_link_libraries(trading_core_lib
        PUBLIC
        common
        data_lib
        PRIVATE
        SQLiteCpp
        sqlite3
)


add_executable(trading_core src/Main.cpp)

target_link_libraries(trading_core
        PUBLIC
        spdlog
        common
        PRIVATE
        trading_core_lib
        data_lib
        SQLiteCpp
        sqlite3
)
