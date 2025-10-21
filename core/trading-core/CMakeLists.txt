cmake_minimum_required(VERSION 3.15)

file(GLOB_RECURSE ALL_CORE_SOURCES
        "src/*.cpp"
        "src/**/*.cpp"
)
list(REMOVE_DUPLICATES ALL_CORE_SOURCES)

list(FILTER ALL_CORE_SOURCES EXCLUDE REGEX ".*/src/app/main.cpp$")
set(LIB_SOURCES ${ALL_CORE_SOURCES})

add_library(trading_core_lib ${LIB_SOURCES})

target_include_directories(trading_core_lib PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)
target_link_libraries(trading_core_lib PUBLIC common)
target_compile_options(trading_core_lib PRIVATE -Wall -Wextra -Wpedantic)

add_executable(trading-core src/app/main.cpp)

target_link_libraries(trading-core PRIVATE trading_core_lib)