#pragma once

#include "common_fix/ExecutionReport.h"
#include <chrono>
#include <map>
#include <shared_mutex>
#include <string>
#include <unordered_set>
#include <vector>

namespace client_blotter {

    struct OrderIntent {
        common::ClientOrderID clientOrderId;
        common::Instrument symbol;
        common::OrderSide side;
        common::OrderType orderType;
        common::Price price;
        common::Quantity originalQuantity;
    };

    struct OrderRow {
        common::ClientOrderID clientOrderId;
        common::Instrument symbol = common::Instrument::EURUSD;
        common::OrderSide side = common::OrderSide::Buy;
        common::OrderType orderType = common::OrderType::Limit;
        bool orderTypeKnown = false;
        common::Price price = 0.0;
        common::Quantity originalQuantity = 0;
        common::Quantity remainingQuantity = 0;
        common::OrderStatus status = common::OrderStatus::New;
        fix::ExchangeOrderID exchangeOrderId = 0;
        std::string reason;
    };

    struct HistoryRow {
        common::ClientOrderID clientOrderId;
        fix::ExchangeOrderID exchangeOrderId = 0;
        common::Instrument symbol = common::Instrument::EURUSD;
        common::OrderSide side = common::OrderSide::Buy;
        common::OrderStatus status = common::OrderStatus::New;
        common::Timestamp timestamp{};
        common::Price lastPrice = 0.0;
        common::Quantity lastQuantity = 0;
        std::string reason;
    };

    /**
     * Thread-safe in-memory order lifecycle and execution history for the
     * client UI.
     *
     * The model is fed by outbound order intents and read-only FIX execution
     * reports. It never sends, cancels, or modifies an order.
     */
    class BlotterModel {
    public:
        void recordOrderIntent(const OrderIntent& intent);
        void processExecution(const fix::ExecutionReport& report);

        [[nodiscard]] std::vector<OrderRow> activeOrders() const;
        [[nodiscard]] std::vector<HistoryRow> history() const;

    private:
        mutable std::shared_mutex mMutex;
        std::map<common::ClientOrderID, OrderRow> mOrders;
        std::vector<HistoryRow> mHistory;
        std::unordered_set<std::string> mSeenExecutionKeys;
    };

} // namespace client_blotter
