cmake_minimum_required(VERSION 3.5)

file(GLOB_RECURSE ALL_DATA_SOURCES "src/*.cpp")
list(FILTER ALL_DATA_SOURCES EXCLUDE REGEX "[/\\]Main\\.cpp$")
list(REMOVE_DUPLICATES ALL_DATA_SOURCES)

add_library(data_lib ${ALL_DATA_SOURCES})

target_include_directories(data_lib PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(data_lib
        PUBLIC common
        PRIVATE SQLiteCpp sqlite3
)

target_compile_options(data_lib PRIVATE -Wall -Wextra -Wpedantic)

add_executable(data src/Main.cpp)
target_link_libraries(data PRIVATE data_lib SQLiteCpp sqlite3)
