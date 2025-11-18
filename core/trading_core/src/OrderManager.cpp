#include "trading_core/OrderManager.h"
#include "logging/Logger.h"
#include <algorithm>

namespace trading_core {

    bool OrderManager::addOrder(std::unique_ptr<common::Order> order)
    {
        if (!order) return false;

        const auto orderId = order->getId();
        if (mOrderMap.count(orderId)) {
            LOG_WARN("Order {} already exists in OrderManager.", orderId);
            return false;
        }

        mOrderMap[orderId] = std::move(order);
        LOG_INFO("Added Order {} to OrderManager.", orderId);
        return true;
    }

    std::optional<common::Order*>
    OrderManager::getOrderById(const common::OrderID& id) const
    {
        auto it = mOrderMap.find(id);
        if (it != mOrderMap.end()) {
            return it->second.get();
        }
        return std::nullopt;
    }

    std::optional<common::Order*>
    OrderManager::getOrderByClientOrderId(const std::string& clOrdId) const
    {
        auto it = std::find_if(mOrderMap.begin(), mOrderMap.end(),
            [&clOrdId](const auto& pair) {
                return std::to_string(pair.second->getClientOrderId()) == clOrdId;
            });

        if (it != mOrderMap.end()) {
            return it->second.get();
        }
        return std::nullopt;
    }

    bool OrderManager::removeOrderById(const common::OrderID& id)
    {
        if (mOrderMap.erase(id)) {
            LOG_INFO("Removed Order {} from OrderManager.", id);
            return true;
        }
        return false;
    }

    bool OrderManager::containsOrderById(const common::OrderID& id) const
    {
        return mOrderMap.count(id) > 0;
    }

    size_t OrderManager::size() const
    {
        return mOrderMap.size();
    }

    const std::unordered_map<common::OrderID, std::unique_ptr<common::Order>>& OrderManager::getOrders() const
    {
        return mOrderMap;
    }

} // namespace trading_core
