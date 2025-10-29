//
// Created by sujal on 21-10-2025.
//

#include "trading_core/OrderManager.h"

namespace trading_core {
    bool OrderManager::addOrder(const common::OrderPtr &order) {
        if (!order) {
            return false;
        }

        const auto [it, status] = mOrderMap.try_emplace(order->getId(), order);
        return status;
    }

    std::optional<common::OrderPtr> OrderManager::getOrderById(const common::OrderID &id) const {
        const auto it = mOrderMap.find(id);

        if (it == mOrderMap.end()) {
            return std::nullopt;
        }

        return it->second;
    }

    bool OrderManager::removeOrderById(const common::OrderID &id) {
        const size_t totalErased = mOrderMap.erase(id);

        return totalErased == 1;
    }

    bool OrderManager::containsOrderById(const common::OrderID &id) const {
        return mOrderMap.contains(id);
    }

    size_t OrderManager::size() const {
        return mOrderMap.size();
    }
}
