//
// Created by sujal on 29-10-2025.
//
#include "trading_core/ExecutionPublisher.h"
#include "logging/Logger.h"
#include <chrono>
#include <string_view>
#include <iomanip>
#include <sstream>

namespace trading_core {
    void ExecutionPublisher::publishExecution(const common::Order &order, const std::string &action) {
        std::stringstream ss;
        ss << "EXECUTION | Action=" << action
           << " | OrderID=" << order.getId()
           << " | Symbol=" << common::to_string(order.getSymbol())
           << " | Qty=" << order.getRemainingQuantity()
           << " | Price=" << std::fixed << std::setprecision(6) << order.getPrice()
           << " | Client=" << order.getClientId();
        LOG_INFO(ss.str());
    }

    void ExecutionPublisher::publishTrade(const common::Trade &trade) {
        auto timestamp_us = std::chrono::duration_cast<std::chrono::microseconds>(
            trade.getTimestamp().time_since_epoch()).count();

        std::stringstream ss;
        ss << "TRADE | TradeID=" << trade.getTradeId()
           << " | BuyOrder=" << trade.getBuyOrderId()
           << " | SellOrder=" << trade.getSellOrderId()
           << " | Qty=" << trade.getQuantity()
           << " | Price=" << std::fixed << std::setprecision(6) << trade.getPrice()
           << " | Timestamp=" << timestamp_us << "us";
        LOG_INFO(ss.str());
    }

    void ExecutionPublisher::publishRejection(const common::OrderID &orderId,
                                              const common::ClientID &clientId,
                                              const std::string_view &reason) {
        std::stringstream ss;
        ss << "REJECT | OrderID=" << orderId
           << " | Client=" << clientId
           << " | Reason=" << reason;
        LOG_INFO(ss.str());
    }
} // namespace trading_core
