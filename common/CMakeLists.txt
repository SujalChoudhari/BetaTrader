cmake_minimum_required(VERSION 3.15)

file(GLOB_RECURSE COMMON_SOURCES "src/*.cpp")
add_library(common ${COMMON_SOURCES})

target_include_directories(common PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(common PUBLIC spdlog)

