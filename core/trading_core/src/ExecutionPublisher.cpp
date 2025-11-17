//
// Created by sujal on 29-10-2025.
//
#include "trading_core/ExecutionPublisher.h"
#include "trading_core/TradingCore.h"
#include "logging/Logger.h"
#include <chrono>
#include <iomanip>
#include <sstream>
#include <string_view>

namespace trading_core {
    void ExecutionPublisher::publishExecution(const common::Order& order,
                                              const std::string& action)
    {
        // Log the internal event
        std::stringstream ss;
        ss << "EXECUTION | Action=" << action << " | OrderID=" << order.getId()
           << " | Client=" << order.getClientId();
        LOG_INFO(ss.str());

        // Determine the correct FIX order status
        common::OrderStatus status = order.getStatus();
        if (action == "NEW") {
            status = common::OrderStatus::New;
        } else if (action == "CANCELED") {
            status = common::OrderStatus::Cancelled;
        }
        // For fills, the order status will already be PartiallyFilled or Filled.

        // Create the definitive FIX ExecutionReport
        // TODO: The hardcoded values for SenderCompID, ExecID, and SeqNum need to be managed properly.
        fix::ExecutionReport report(
            static_cast<fix::CompID>(1), // SenderCompID (Server)
            static_cast<fix::CompID>(std::stoul(order.getClientId())), // TargetCompID (Client Session ID)
            static_cast<fix::SequenceNumber>(0), // MsgSeqNum - Should be managed per session
            static_cast<fix::ExchangeOrderID>(order.getId()), // ExchangeOrderID
            static_cast<fix::ClientOrderID>(std::stoul(order.getClientId())), // ClientOrderID
            "exec_id", // ExecutionID - Should be unique per execution
            status,
            action,
            order.getSymbol(),
            order.getSide(),
            static_cast<fix::Quantity>(order.getOriginalQuantity()),
            static_cast<fix::Quantity>(order.getOriginalQuantity() - order.getRemainingQuantity()), // CumQty
            static_cast<fix::Quantity>(order.getRemainingQuantity()), // LeavesQty
            static_cast<fix::Price>(order.getPrice()), // LastPx
            static_cast<fix::Quantity>(0), // LastQty - Should be the quantity of this specific execution
            order.getTimestamp()
        );

        // Publish the report to subscribers
        auto& core = TradingCore::getInstance();
        if (auto& callback = core.getExecutionReportCallback()) {
            callback(report);
        }
    }

    void ExecutionPublisher::publishTrade(const common::Trade& trade)
    {
        // This function should also create and publish a fill ExecutionReport
        // For now, it only logs.
        // TODO: Implement trade-to-execution-report logic.
        auto timestamp_us
                = std::chrono::duration_cast<std::chrono::microseconds>(
                          trade.getTimestamp().time_since_epoch())
                          .count();

        std::stringstream ss;
        ss << "TRADE | TradeID=" << trade.getTradeId()
           << " | BuyOrder=" << trade.getBuyOrderId()
           << " | SellOrder=" << trade.getSellOrderId()
           << " | Qty=" << trade.getQuantity() << " | Price=" << std::fixed
           << std::setprecision(6) << trade.getPrice()
           << " | Timestamp=" << timestamp_us << "us";
        LOG_INFO(ss.str());
    }

    void ExecutionPublisher::publishRejection(const common::OrderID& orderId,
                                              const common::ClientID& clientId,
                                              const std::string_view& reason)
    {
        // This function should also create and publish a reject ExecutionReport
        // For now, it only logs.
        // TODO: Implement rejection-to-execution-report logic.
        std::stringstream ss;
        ss << "REJECT | OrderID=" << orderId << " | Client=" << clientId
           << " | Reason=" << reason;
        LOG_INFO(ss.str());
    }
} // namespace trading_core
