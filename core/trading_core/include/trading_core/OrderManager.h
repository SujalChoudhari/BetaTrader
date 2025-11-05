//
// Created by sujal on 21-10-2025.
//
#pragma once
#include <unordered_map>
#include <memory>

#include "common/Order.h"
#include "common/Types.h"


namespace trading_core {
    /**
     * @class OrderManager
     * @brief Manages all the orders in the system.
     *
     * This class is responsible for adding, removing, and retrieving orders. It stores
     * all orders in an unordered map, using the order ID as the key.
     */
    class OrderManager {
    public:
        virtual ~OrderManager() = default;

        /**
         * @brief Default constructor.
         */
        OrderManager() = default;

    public:
        /**
         * @brief Adds an order to the order manager.
         * @param order A unique pointer to the order to be added.
         * @return True if the order was successfully added, false otherwise.
         */
        virtual bool addOrder(std::unique_ptr<common::Order> order);

        /**
         * @brief Gets an order by its ID.
         * @param id The ID of the order to retrieve.
         * @return An optional containing a raw pointer to the order if it was found, otherwise an empty optional.
         */
        [[nodiscard]] virtual std::optional<common::Order*> getOrderById(const common::OrderID &id) const;

        /**
         * @brief Removes an order by its ID.
         * @param id The ID of the order to be removed.
         * @return True if the order was successfully removed, false otherwise.
         */
        virtual bool removeOrderById(const common::OrderID &id);

        /**
         * @brief Checks if an order exists in the order manager.
         * @param id The ID of the order to check.
         * @return True if the order exists, false otherwise.
         */
        bool containsOrderById(const common::OrderID &id) const;

        /**
         * @brief Gets the number of orders in the order manager.
         * @return The number of orders.
         */
        size_t size() const;

    private:
        std::unordered_map<common::OrderID, std::unique_ptr<common::Order>> mOrderMap; ///< A map of order IDs to orders.
    };
}
