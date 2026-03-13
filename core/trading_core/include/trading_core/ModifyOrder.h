//
// Created by sujal on 21-10-2025.
//

#pragma once
#include "common/Time.h"
#include "common/Types.h"
#include "trading_core/Command.h"
#include "common_trading/CommandType.h"

namespace trading_core {
    /**
     * @class ModifyOrder
     * @brief A command to modify an existing order in the trading engine.
     *
     * This class derives from the base Command class and encapsulates the data
     * required to modify an order, including the order ID, new price, and new
     * quantity.
     */
    class ModifyOrder : public Command {
    public:
        /**
         * @brief Constructs a new ModifyOrder command.
         * @param clientId The identifier of the client modifying the order.
         * @param timestamp The time at which the command was created.
         * @param orderId The ID of the order to be modified.
         * @param newPrice The new price for the order.
         * @param newQuantity The new quantity for the order.
         */
        explicit ModifyOrder(const common::ClientID& clientId,
                             const common::Timestamp timestamp,
                             const common::OrderID orderId,
                             const common::Price newPrice,
                             const common::Quantity newQuantity)
            : Command(CommandType::ModifyOrder, clientId, timestamp),
              mOrderId(orderId), mNewPrice(newPrice),
              mNewQuantity(newQuantity) {};

        /** @brief Gets the ID of the order to be modified. */
        [[nodiscard]] common::OrderID getOrderId() const { return mOrderId; }
        /** @brief Gets the new price for the order. */
        [[nodiscard]] const common::Price& getNewPrice() const
        {
            return mNewPrice;
        }
        /** @brief Gets the new quantity for the order. */
        [[nodiscard]] const common::Quantity& getNewQuantity() const
        {
            return mNewQuantity;
        }

    private:
        common::OrderID mOrderId; ///< The ID of the order to be modified.
        common::Price mNewPrice; ///< The new price for the order.
        common::Quantity mNewQuantity; ///< The new quantity for the order.
    };
} // namespace trading_core
