add_library(exchange_state_lib
    src/OrderManager.cpp
    src/OrderIDGenerator.cpp
    src/TradeIDGenerator.cpp
)
target_include_directories(exchange_state_lib PUBLIC include)
target_link_libraries(exchange_state_lib PUBLIC common data_lib)
