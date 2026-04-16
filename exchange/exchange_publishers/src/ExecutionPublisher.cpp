#include <exchange_publishers/ExecutionPublisher.h>
#include <iostream>
#include "logging/Logger.h"
#include <chrono>
#include <iomanip>
#include <sstream>
#include <string_view>

namespace trading_core {

    ExecutionPublisher::ExecutionReportCallback ExecutionPublisher::s_callback = nullptr;
    std::mutex ExecutionPublisher::s_mutex;

    void ExecutionPublisher::SetCallback(ExecutionReportCallback callback)
    {
        std::lock_guard<std::mutex> lock(s_mutex);
        s_callback = std::move(callback);
    }

    uint32_t stoul_safe_ep(const std::string& s)
    {
        try {
            if (s.empty()) return 0;
            return std::stoul(s);
        }
        catch (...) {
            return 0;
        }
    }

    void ExecutionPublisher::publishExecution(const common::Order& order, const std::string& action)
    {
        LOG_INFO("EXECUTION | Action={} | OrderID={} | Symbol={} | Qty={} | Price={} | Client={}",
                 action, order.getId(), common::to_string(order.getSymbol()),
                 order.getRemainingQuantity(), order.getPrice(), order.getClientId());

        common::OrderStatus status = (action == "NEW") ? common::OrderStatus::New : order.getStatus();
        if (action == "CANCELED") {
            status = common::OrderStatus::Cancelled;
        }

        fix::ExecutionReport report(
            static_cast<fix::CompID>(1),
            static_cast<fix::CompID>(stoul_safe_ep(order.getClientId())),
            static_cast<fix::SequenceNumber>(0),
            order.getId(),
            order.getClientOrderId(),
            "exec_" + std::to_string(order.getId()),
            status,
            action,
            order.getSymbol(),
            order.getSide(),
            order.getOriginalQuantity(),
            order.getOriginalQuantity() - order.getRemainingQuantity(),
            order.getRemainingQuantity(),
            order.getPrice(),
            0,
            order.getTimestamp()
        );

        ExecutionReportCallback cb;
        {
            std::lock_guard<std::mutex> lock(s_mutex);
            cb = s_callback;
        }
        if (cb) {
            cb(report);
        }
    }

    void ExecutionPublisher::publishTrade(const common::Trade& trade, const common::Order& buyOrder, const common::Order& sellOrder)
    {
        LOG_INFO("TRADE | TradeID={} | BuyOrder={} | SellOrder={} | Qty={} | Price={}",
                 trade.getTradeId(), trade.getBuyOrderId(), trade.getSellOrderId(),
                 trade.getQuantity(), trade.getPrice());

        fix::ExecutionReport buyReport(
            static_cast<fix::CompID>(1),
            static_cast<fix::CompID>(stoul_safe_ep(buyOrder.getClientId())),
            0,
            buyOrder.getId(),
            buyOrder.getClientOrderId(),
            "exec_trade_" + std::to_string(trade.getTradeId()) + "_buy",
            buyOrder.getStatus(),
            "TRADE",
            buyOrder.getSymbol(),
            buyOrder.getSide(),
            buyOrder.getOriginalQuantity(),
            buyOrder.getOriginalQuantity() - buyOrder.getRemainingQuantity(),
            buyOrder.getRemainingQuantity(),
            trade.getPrice(),
            trade.getQuantity(),
            trade.getTimestamp()
        );

        fix::ExecutionReport sellReport(
            static_cast<fix::CompID>(1),
            static_cast<fix::CompID>(stoul_safe_ep(sellOrder.getClientId())),
            0,
            sellOrder.getId(),
            sellOrder.getClientOrderId(),
            "exec_trade_" + std::to_string(trade.getTradeId()) + "_sell",
            sellOrder.getStatus(),
            "TRADE",
            sellOrder.getSymbol(),
            sellOrder.getSide(),
            sellOrder.getOriginalQuantity(),
            sellOrder.getOriginalQuantity() - sellOrder.getRemainingQuantity(),
            sellOrder.getRemainingQuantity(),
            trade.getPrice(),
            trade.getQuantity(),
            trade.getTimestamp()
        );

        ExecutionReportCallback cb;
        {
            std::lock_guard<std::mutex> lock(s_mutex);
            cb = s_callback;
        }
        if (cb) {
            cb(buyReport);
            cb(sellReport);
        }
    }

    void ExecutionPublisher::publishRejection(const common::OrderID& orderId, const common::ClientID& clientId, const common::Symbol& symbol, const common::OrderSide& side, const std::string_view& reason)
    {
        LOG_INFO("REJECT | ClOrdID={} | Client={} | Reason={}", orderId, clientId, reason);
        
        fix::ExecutionReport report(
            static_cast<fix::CompID>(1),
            static_cast<fix::CompID>(stoul_safe_ep(std::string(clientId))),
            0,
            orderId,
            static_cast<fix::ClientOrderID>(orderId), // Fallback to orderId as clientOrderId if unknown
            "exec_reject_" + std::to_string(orderId),
            common::OrderStatus::Rejected,
            std::string(reason),
            symbol,
            side,
            0,
            0,
            0,
            0,
            0,
            std::chrono::system_clock::now()
        );
        
        ExecutionReportCallback cb;
        {
            std::lock_guard<std::mutex> lock(s_mutex);
            cb = s_callback;
        }
        if (cb) {
            cb(report);
        }
    }
} // namespace trading_core
