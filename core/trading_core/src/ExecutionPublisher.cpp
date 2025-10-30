//
// Created by sujal on 29-10-2025.
//
#include "trading_core/ExecutionPublisher.h"
#include <iostream> // For std::cout
#include <chrono> // For std::chrono
#include <string_view> // For std::string_view

namespace trading_core {
    void ExecutionPublisher::publishExecution(const common::Order &order, const std::string &action) {
        std::cout << "[ExecutionPublisher] EXECUTION | Action=" << action
                  << " | OrderID=" << order.getId()
                  << " | Symbol=" << common::to_string(order.getSymbol())
                  << " | Qty=" << order.getRemainingQuantity()
                  << " | Price=" << order.getPrice()
                  << " | Client=" << order.getClientId() << std::endl;
    }

    void ExecutionPublisher::publishTrade(const common::Trade &trade) {
        auto timestamp_us = std::chrono::duration_cast<std::chrono::microseconds>(trade.getTimestamp().time_since_epoch()).count();

        std::cout << "[ExecutionPublisher] TRADE | TradeID=" << trade.getTradeId()
                  << " | BuyOrder=" << trade.getBuyOrderId()
                  << " | SellOrder=" << trade.getSellOrderId()
                  << " | Qty=" << trade.getQuantity()
                  << " | Price=" << trade.getPrice()
                  << " | Timestamp=" << timestamp_us << "us" << std::endl;
    }

    void ExecutionPublisher::publishRejection(const common::OrderID &orderId,
                                              const common::ClientID &clientId,
                                              const std::string_view &reason) {
        std::cout << "[ExecutionPublisher] REJECT | OrderID=" << orderId
                  << " | Client=" << clientId
                  << " | Reason=" << reason << std::endl;
    }
} // namespace trading_core
