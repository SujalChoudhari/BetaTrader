add_library(exchange_routing_lib
    src/Partition.cpp
    src/WorkerThread.cpp
)
target_include_directories(exchange_routing_lib PUBLIC include)
target_link_libraries(exchange_routing_lib common exchange_matching_lib exchange_state_lib exchange_risk_lib exchange_publishers_lib data_lib)
