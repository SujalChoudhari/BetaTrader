#include "trading_core/ExecutionPublisher.h"
#include <iostream>
#include "trading_core/TradingCore.h"
#include "trading_core/Partition.h"
#include "trading_core/OrderManager.h"
#include "logging/Logger.h"
#include <chrono>
#include <iomanip>
#include <sstream>
#include <string_view>

namespace trading_core {

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
            static_cast<fix::CompID>(std::stoul(order.getClientId())),
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

        auto& core = TradingCore::getInstance();
        if (auto& callback = core.getExecutionReportCallback()) {
            callback(report);
        }
    }

    void ExecutionPublisher::publishTrade(const common::Trade& trade)
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

        auto& core = TradingCore::getInstance();

        auto buyOrderOpt = core.getOrder(trade.getBuyOrderId());
        auto sellOrderOpt = core.getOrder(trade.getSellOrderId());

        if (!buyOrderOpt || !sellOrderOpt) {
            LOG_ERROR("Could not find one or both orders for trade {}", trade.getTradeId());
            return;
        }
        
        const auto& buyOrder = *buyOrderOpt;
        const auto& sellOrder = *sellOrderOpt;

        fix::ExecutionReport buyReport(
            static_cast<fix::CompID>(1),
            static_cast<fix::CompID>(std::stoul(buyOrder.getClientId())),
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
            static_cast<fix::CompID>(std::stoul(sellOrder.getClientId())),
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

        if (auto& callback = core.getExecutionReportCallback()) {
            callback(buyReport);
            callback(sellReport);
        }
    }

    void ExecutionPublisher::publishRejection(const common::OrderID& clOrdId, const common::ClientID& clientId, const std::string_view& reason)
    {
        std::stringstream ss;
        ss << "REJECT | OrderID=" << clOrdId << " | Client=" << clientId << " | Reason=" << reason;
        std::cout << "[ExecutionPublisher] " << ss.str() << std::endl;
        LOG_INFO("REJECT | ClOrdID={} | Client={} | Reason={}", clOrdId, clientId, reason);
        
        auto& core = TradingCore::getInstance();
        auto orderOpt = core.getOrderByClientOrderId(std::to_string(clOrdId));

        if (!orderOpt) {
            LOG_ERROR("Could not find order with ClOrdID {} to publish rejection.", clOrdId);
            return;
        }
        const auto& order = *orderOpt;

        fix::ExecutionReport report(
            static_cast<fix::CompID>(1),
            static_cast<fix::CompID>(std::stoul(std::string(clientId))),
            0,
            order.getId(),
            order.getClientOrderId(),
            "exec_reject_" + std::to_string(order.getId()),
            common::OrderStatus::Rejected,
            "REJECTED",
            order.getSymbol(),
            order.getSide(),
            order.getOriginalQuantity(),
            order.getOriginalQuantity() - order.getRemainingQuantity(),
            order.getRemainingQuantity(),
            order.getPrice(),
            0,
            order.getTimestamp()
        );
        
        if (auto& callback = core.getExecutionReportCallback()) {
            callback(report);
        }
    }
} // namespace trading_core
