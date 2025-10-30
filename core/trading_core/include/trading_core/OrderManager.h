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
     * Represents the Manager for orders
     *
     * Handles functionality of adding orders and removing them.
     * Stores a map of all orders with their orderId.
     */
    class OrderManager {
    public:
        OrderManager() = default;

    public:
        bool addOrder(std::shared_ptr<common::Order> order);

        [[nodiscard]] std::optional<std::shared_ptr<common::Order>> getOrderById(const common::OrderID &id) const;

        bool removeOrderById(const common::OrderID &id);

        bool containsOrderById(const common::OrderID &id) const;

        size_t size() const;

    private:
        std::unordered_map<common::OrderID, std::shared_ptr<common::Order>> mOrderMap;
    };
}
