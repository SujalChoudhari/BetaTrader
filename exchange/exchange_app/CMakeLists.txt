add_library(exchange_app_lib
    src/TradingCore.cpp
)
target_include_directories(exchange_app_lib PUBLIC include)
target_link_libraries(exchange_app_lib 
    PUBLIC 
    exchange_state_lib
    exchange_publishers_lib
    exchange_routing_lib
)
