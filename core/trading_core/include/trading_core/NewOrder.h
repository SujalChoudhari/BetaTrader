//
// Created by sujal on 21-10-2025.
//

#pragma once
#include "Command.h"
#include "common/Order.h"
#include <memory>

namespace trading_core {
    /**
     * @class NewOrder
     * @brief A command to submit a new order to the trading engine.
     *
     * This class derives from the base Command class and encapsulates the data
     * required to create a new order, including the client ID, timestamp, and a unique
     * pointer to the Order object.
     */
    class NewOrder : public Command {
    public:
        /**
         * @brief Constructs a new NewOrder command.
         * @param clientId The identifier of the client submitting the order.
         * @param timestamp The time at which the command was created.
         * @param order A unique pointer to the common::Order object being submitted.
         */
        explicit NewOrder(const common::ClientID &clientId, const common::Timestamp timestamp,
                          std::unique_ptr<common::Order> order) : Command(CommandType::NewOrder, clientId, timestamp),
                                                 mOrder(std::move(order)) {
        };

        /**
         * @brief Gets the order associated with this command.
         * @return A raw pointer to the common::Order object.
         */
        [[nodiscard]] common::Order* getOrder() const { return mOrder.get(); }

        /**
         * @brief Releases ownership of the order.
         * @return A unique pointer to the common::Order object.
         */
        [[nodiscard]] std::unique_ptr<common::Order> releaseOrder() { return std::move(mOrder); }

    private:
        std::unique_ptr<common::Order> mOrder; ///< A unique pointer to the new order.
    };
}
