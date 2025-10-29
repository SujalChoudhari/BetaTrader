//
// Created by sujal on 29-10-2025.
//

#include "trading_core/ExecutionPublisher.h"

#include "logging/Logger.h"

namespace trading_core {
    void ExecutionPublisher::publishExecution(const common::Order &order, const std::string &action) {
        LOG_INFO("[ExecutionPublisher] EXECUTION | Action={} | OrderID={} | Symbol={} | Qty={} | Price={} | Client={}",
                 action,
                 order.getId(),
                 common::to_string(order.getSymbol()),
                 order.getRemainingQuantity(),
                 order.getPrice(),
                 order.getClientId());
    }

    void ExecutionPublisher::publishTrade(const common::Trade &trade) {
        auto timestamp_us = duration_cast<std::chrono::microseconds>(trade.getTimestamp().time_since_epoch()).count();

        LOG_INFO(
            "[ExecutionPublisher] TRADE | TradeID={} | BuyOrder={} | SellOrder={} | Qty={} | Price={} | Timestamp={}us",
            trade.getTradeId(),
            trade.getBuyOrderId(),
            trade.getSellOrderId(),
            trade.getQuantity(),
            trade.getPrice(),
            timestamp_us);
    }

    void ExecutionPublisher::publishRejection(const common::OrderID &orderId,
                                              const common::ClientID &clientId,
                                              const std::string_view &reason) {
        LOG_INFO("[ExecutionPublisher] REJECT | OrderID={} | Client={} | Reason={}",
                 orderId,
                 clientId,
                 reason);
    }
} // namespace trading_core
