#include <exchange_publishers/ExecutionPublisher.h>
#include <iostream>
#include "logging/Logger.h"
#include <chrono>
#include <iomanip>
#include <sstream>
#include <string_view>

namespace trading_core {

    ExecutionPublisher::ExecutionReportCallback ExecutionPublisher::s_callback = nullptr;

    void ExecutionPublisher::SetCallback(ExecutionReportCallback callback)
    {
        s_callback = callback;
    }

    uint32_t stoul_safe(const std::string& s)
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
        std::stringstream ss;
        ss << "EXECUTION | Action=" << action << " | OrderID=" << order.getId()
           << " | Symbol=" << common::to_string(order.getSymbol())
           << " | Qty=" << order.getRemainingQuantity()
           << " | Price=" << std::fixed << std::setprecision(6) << order.getPrice()
           << " | Client=" << order.getClientId();
        std::cout << "[ExecutionPublisher] " << ss.str() << std::endl;
        LOG_INFO(ss.str());

        common::OrderStatus status = (action == "NEW") ? common::OrderStatus::New : order.getStatus();
        if (action == "CANCELED") {
            status = common::OrderStatus::Cancelled;
        }

        fix::ExecutionReport report(
            static_cast<fix::CompID>(1),
            static_cast<fix::CompID>(stoul_safe(order.getClientId())),
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

        if (s_callback) {
            s_callback(report);
        }
    }

    void ExecutionPublisher::publishTrade(const common::Trade& trade, const common::Order& buyOrder, const common::Order& sellOrder)
    {
        std::stringstream ss;
        ss << "TRADE | TradeID=" << trade.getTradeId()
           << " | BuyOrder=" << trade.getBuyOrderId()
           << " | SellOrder=" << trade.getSellOrderId()
           << " | Qty=" << trade.getQuantity()
           << " | Price=" << std::fixed << std::setprecision(6) << trade.getPrice()
           << " | Timestamp=" << std::chrono::duration_cast<std::chrono::microseconds>(trade.getTimestamp().time_since_epoch()).count() << "us";
        std::cout << "[ExecutionPublisher] " << ss.str() << std::endl;
        LOG_INFO("TRADE | TradeID={} | BuyOrder={} | SellOrder={} | Qty={} | Price={}",
                 trade.getTradeId(), trade.getBuyOrderId(), trade.getSellOrderId(),
                 trade.getQuantity(), trade.getPrice());

        fix::ExecutionReport buyReport(
            static_cast<fix::CompID>(1),
            static_cast<fix::CompID>(stoul_safe(buyOrder.getClientId())),
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
            static_cast<fix::CompID>(stoul_safe(sellOrder.getClientId())),
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

        if (s_callback) {
            s_callback(buyReport);
            s_callback(sellReport);
        }
    }

    void ExecutionPublisher::publishRejection(const common::OrderID& orderId, const common::ClientID& clientId, const common::Symbol& symbol, const common::OrderSide& side, const std::string_view& reason)
    {
        std::stringstream ss;
        ss << "REJECT | OrderID=" << orderId << " | Client=" << clientId << " | Reason=" << reason;
        std::cout << "[ExecutionPublisher] " << ss.str() << std::endl;
        LOG_INFO("REJECT | ClOrdID={} | Client={} | Reason={}", orderId, clientId, reason);
        
        fix::ExecutionReport report(
            static_cast<fix::CompID>(1),
            static_cast<fix::CompID>(stoul_safe(std::string(clientId))),
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
        
        if (s_callback) {
            s_callback(report);
        }
    }
} // namespace trading_core
