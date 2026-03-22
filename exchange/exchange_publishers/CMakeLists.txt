add_library(exchange_publishers_lib
    src/ExecutionPublisher.cpp
    src/MarketDataPublisher.cpp
)
target_include_directories(exchange_publishers_lib PUBLIC include)
target_link_libraries(exchange_publishers_lib common)
