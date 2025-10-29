//
// Created by sujal on 29-10-2025.
//
#pragma once

#include <string>
#include "common/Order.h"
#include "common/Trade.h"

namespace trading_core {
    class ExecutionPublisher {
    public:
        ExecutionPublisher() = default;

        ~ExecutionPublisher() = default;

        ExecutionPublisher(const ExecutionPublisher &) = delete;

        ExecutionPublisher &operator=(const ExecutionPublisher &) = delete;

        // Log order acknowledgment (e.g., new, cancel, modify)
        static void publishExecution(const common::Order &order, const std::string &action);

        // Log executed trade
        static void publishTrade(const common::Trade &trade);

        // Log rejection or error
        static void publishRejection(const common::OrderID &orderId,
                                     const common::ClientID &clientId,
                                     const std::string_view &reason);
    };
} // namespace trading_core
