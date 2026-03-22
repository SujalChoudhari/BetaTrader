cmake_minimum_required(VERSION 3.15)

file(GLOB_RECURSE ALL_DATA_SOURCES "src/*.cpp")
list(FILTER ALL_DATA_SOURCES EXCLUDE REGEX "[/\\]Main\\.cpp$")
list(REMOVE_DUPLICATES ALL_DATA_SOURCES)

add_library(data_lib ${ALL_DATA_SOURCES})

target_include_directories(data_lib PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${CMAKE_SOURCE_DIR}/vendor/lib/SPSCQueue/include
)

target_link_libraries(data_lib
        PUBLIC common
        PUBLIC SQLiteCpp sqlite3
)
