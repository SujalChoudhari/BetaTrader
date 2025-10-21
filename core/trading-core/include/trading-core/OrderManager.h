//
// Created by sujal on 21-10-2025.
//
#pragma once
#include <unordered_map>

#include "trading-core/Order.h"
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
        bool addOrder(const OrderPtr &order);

        std::optional<OrderPtr> getOrderById(const common::OrderID &id) const;

        bool removeOrderById(const common::OrderID &id);

        bool containsOrderById(const common::OrderID &id) const;

        size_t size() const;

    private:
        std::unordered_map<common::OrderID, OrderPtr> mOrderMap;
    };
}
