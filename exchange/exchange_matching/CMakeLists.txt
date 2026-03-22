add_library(exchange_matching_lib
    src/Matcher.cpp
    src/OrderBook.cpp
)
target_include_directories(exchange_matching_lib PUBLIC include)
target_link_libraries(exchange_matching_lib PUBLIC common exchange_state_lib exchange_publishers_lib)
