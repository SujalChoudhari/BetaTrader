cmake_minimum_required(VERSION 3.15)

file(GLOB_RECURSE ALL_FIX_SOURCES "src/*.cpp")
list(REMOVE_DUPLICATES ALL_FIX_SOURCES)

add_library(fix_lib ${ALL_FIX_SOURCES})

target_include_directories(fix_lib PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(fix_lib
        PUBLIC
        common
)
