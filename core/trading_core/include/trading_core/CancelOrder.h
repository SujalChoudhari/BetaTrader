//
// Created by sujal on 21-10-2025.
//

#pragma once
#include "common/Order.h"
#include "trading_core/Command.h"
#include "common_trading/CommandType.h"

namespace trading_core {
    /**
     * @class CancelOrder
     * @brief A command to cancel an existing order in the trading engine.
     *
     * This class derives from the base Command class and encapsulates the data
     * required to cancel an order, including the order ID.
     */
    class CancelOrder : public Command {
    public:
        /**
         * @brief Constructs a new CancelOrder command.
         * @param clientId The identifier of the client cancelling the order.
         * @param timestamp The time at which the command was created.
         * @param orderId The ID of the order to be cancelled.
         */
        explicit CancelOrder(const common::ClientID& clientId,
                             const common::Timestamp timestamp,
                             const common::OrderID orderId)
            : Command(CommandType::CancelOrder, clientId, timestamp),
              mOrderId(orderId) {};

        /** @brief Gets the ID of the order to be cancelled. */
        [[nodiscard]] common::OrderID getOrderId() const { return mOrderId; }

    private:
        common::OrderID mOrderId; ///< The ID of the order to be cancelled.
    };
} // namespace trading_core
