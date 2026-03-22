//
// Created by sujal on 29-10-2025.
//
#pragma once

#include "common/Order.h"
#include "common/Trade.h"
#include "common_fix/ExecutionReport.h" // Include the report type
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
        /**
         * @brief Default constructor is deleted to prevent instantiation.
         */
        ExecutionPublisher() = default;

        /**
         * @brief Default destructor.
         */
        ~ExecutionPublisher() = default;

        /**
         * @brief Copy constructor is deleted to prevent copying.
         */
        ExecutionPublisher(const ExecutionPublisher&) = delete;

        /**
         * @brief Copy assignment operator is deleted to prevent copying.
         */
        ExecutionPublisher& operator=(const ExecutionPublisher&) = delete;

        /**
         * @brief Publishes an execution report for an order.
         * @param order The order for which the execution report is being
         * published.
         * @param action The action that was taken on the order (e.g., "NEW",
         * "CANCELED").
         */
        static void publishExecution(const common::Order& order,
                                     const std::string& action);

        /**
         * @brief Publishes a trade execution.
         * @param trade The trade that was executed.
         */
        static void publishTrade(const common::Trade& trade);

        /**
         * @brief Publishes a rejection for an order.
         * @param orderId The ID of the order that was rejected.
         * @param clientId The ID of the client who submitted the order.
         * @param reason The reason for the rejection.
         */
        static void publishRejection(const common::OrderID& orderId,
                                     const common::ClientID& clientId,
                                     const std::string_view& reason);
    };
} // namespace trading_core
