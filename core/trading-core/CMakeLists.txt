cmake_minimum_required(VERSION 3.15)

file(GLOB_RECURSE CORE_SOURCES
        "src/**/*.cpp"
)

add_executable(trading-core ${CORE_SOURCES})

target_include_directories(trading-core PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(trading-core PRIVATE common)

target_compile_options(trading-core PRIVATE -Wall -Wextra -Wpedantic)

