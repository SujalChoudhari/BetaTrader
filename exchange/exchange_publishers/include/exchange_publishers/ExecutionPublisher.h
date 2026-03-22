//
// Created by sujal on 29-10-2025.
//
#pragma once

#include "common/Order.h"
#include "common/Trade.h"
#include "common_fix/ExecutionReport.h" // Include the report type
#include <functional>
#include <string>

namespace trading_core {
    /**
     * @class ExecutionPublisher
     * @brief A utility class for publishing execution reports, trades, and
     * rejections.
     *
     * This class provides a centralized way to log and publish information
     * about the state of orders and trades in the system. It is designed as a
     * static utility class and cannot be instantiated.
     */
    class ExecutionPublisher {
    public:
        using ExecutionReportCallback = std::function<void(const fix::ExecutionReport&)>;

        static void SetCallback(ExecutionReportCallback callback);

        static void publishExecution(const common::Order& order,
                                     const std::string& action);

        static void publishTrade(const common::Trade& trade,
                                 const common::Order& buyOrder,
                                 const common::Order& sellOrder);

        static void publishRejection(const common::OrderID& orderId,
                                     const common::ClientID& clientId,
                                     const common::Symbol& symbol,
                                     const common::OrderSide& side,
                                     const std::string_view& reason);
    private:
        static ExecutionReportCallback s_callback;
    };
} // namespace trading_core
