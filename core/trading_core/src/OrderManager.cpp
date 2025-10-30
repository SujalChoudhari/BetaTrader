#include "trading_core/OrderManager.h"

#include "logging/Logger.h"
#include "trading_core/TradingCoreRunbookDefinations.h"

namespace trading_core {
    bool OrderManager::addOrder(std::shared_ptr<common::Order> order) {
        if (!order) {
            LOG_ERROR(errors::ETRADE4, "Attempted to add a null order.");
            return false;
        }

        const auto [it, status] = mOrderMap.try_emplace(order->getId(), order);
        if (status) {
            LOG_INFO("Added Order {} to OrderManager.", order->getId());
        } else {
            LOG_WARN("Order {} already exists in OrderManager.", order->getId());
        }
        return status;
    }

    std::optional<std::shared_ptr<common::Order>> OrderManager::getOrderById(const common::OrderID &id) const {
        const auto it = mOrderMap.find(id);

        if (it == mOrderMap.end()) {
            LOG_ERROR(errors::ETRADE6, "Order with ID {} not found in OrderManager.", id);
            return std::nullopt;
        }
        LOG_INFO("Retrieved order with ID {} from OrderManager.", id);
        return it->second;
    }

    bool OrderManager::removeOrderById(const common::OrderID &id) {
        const size_t totalErased = mOrderMap.erase(id);

        if (totalErased == 1) {
            LOG_INFO("Removed order with ID {} from OrderManager.", id);
            return true;
        } else {
            LOG_ERROR(errors::ETRADE6, "Failed to remove order with ID {} from OrderManager. Order not found or multiple orders removed.", id);
            return false;
        }
    }

    bool OrderManager::containsOrderById(const common::OrderID &id) const {
        bool contains = mOrderMap.contains(id);
        LOG_DEBUG("OrderManager {} order with ID {}.", contains ? "contains" : "does not contain", id);
        return contains;
    }

    size_t OrderManager::size() const {
        LOG_DEBUG("OrderManager current size: {}.", mOrderMap.size());
        return mOrderMap.size();
    }
}
