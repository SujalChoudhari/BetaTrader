cmake_minimum_required(VERSION 3.5)

file(GLOB_RECURSE ALL_CORE_SOURCES
        "src/*.cpp"
)
list(FILTER ALL_CORE_SOURCES EXCLUDE REGEX "[/\\]Main\\.cpp$")
list(REMOVE_DUPLICATES ALL_CORE_SOURCES)

add_library(trading_core_lib ${ALL_CORE_SOURCES})

target_include_directories(trading_core_lib PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(trading_core_lib
        PUBLIC common
        PRIVATE SQLiteCpp sqlite3
)

target_compile_options(trading_core_lib PRIVATE -Wall -Wextra -Wpedantic)

add_executable(trading-core src/Main.cpp)
target_link_libraries(trading-core PRIVATE trading_core_lib SQLiteCpp sqlite3)
