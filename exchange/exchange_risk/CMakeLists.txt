add_library(exchange_risk_lib
    src/RiskManager.cpp
)
target_include_directories(exchange_risk_lib PUBLIC include)
target_link_libraries(exchange_risk_lib PUBLIC common exchange_matching_lib data_lib)
