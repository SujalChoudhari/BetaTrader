#include "trading_core/RiskManager.h"
#include "logging/Logger.h"

namespace trading_core {

    constexpr double MAX_PRICE_DEVIATION = 0.10; // 10% deviation

    // Helper function to check for self-match
    template<typename TMap>
    bool checkForSelfMatch(const common::Order& order, const TMap* oppositeMap)
    {
        if (!oppositeMap->empty()) {
            const auto& bestPriceLevel = oppositeMap->begin()->second;
            if (!bestPriceLevel.empty()) {
                const auto& bestOrder = bestPriceLevel.front();
                if (bestOrder->getClientId() == order.getClientId()) {
                    if (order.getOrderType() == common::OrderType::Market) {
                        LOG_ERROR("ETRADE12",
                                  "Pre-check failed for order ID {}: "
                                  "Self-match detected with market order.",
                                  order.getId());
                        return true; // Self-match detected
                    }
                    else if (order.getSide() == common::OrderSide::Buy
                             && order.getPrice() >= bestOrder->getPrice()) {
                        LOG_ERROR("ETRADE12",
                                  "Pre-check failed for order ID {}: "
                                  "Self-match detected with limit order.",
                                  order.getId());
                        return true; // Self-match detected
                    }
                    else if (order.getSide() == common::OrderSide::Sell
                             && order.getPrice() <= bestOrder->getPrice()) {
                        LOG_ERROR("ETRADE12",
                                  "Pre-check failed for order ID {}: "
                                  "Self-match detected with limit order.",
                                  order.getId());
                        return true; // Self-match detected
                    }
                }
            }
        }
        return false; // No self-match
    }

    RiskManager::RiskManager(data::TradeRepository* tradeRepository)
        : mTradeRepository(tradeRepository)
    {
        LOG_INFO("RiskManager initialized.");
    }

    bool RiskManager::preCheck(const common::Order& order, OrderBook& orderBook)
    {
        // Basic sanity checks
        if (order.getOriginalQuantity() <= 0
            || (order.getOrderType() == common::OrderType::Limit
                && order.getPrice() <= 0)) {
            LOG_ERROR("ETRADE10",
                      "Pre-check failed for order ID {}: Invalid quantity or "
                      "price.",
                      order.getId());
            return false;
        }

        // Fat-finger check for limit orders
        if (order.getOrderType() == common::OrderType::Limit) {
            common::Price topOfBookPrice = 0;
            if (order.getSide() == common::OrderSide::Buy
                && !orderBook.getAskMap()->empty()) {
                topOfBookPrice = orderBook.getAskMap()->begin()->first;
            }
            else if (order.getSide() == common::OrderSide::Sell
                     && !orderBook.getBidMap()->empty()) {
                topOfBookPrice = orderBook.getBidMap()->begin()->first;
            }

            if (topOfBookPrice > 0) {
                double deviation = std::abs(order.getPrice() - topOfBookPrice)
                                   / topOfBookPrice;
                if (deviation > MAX_PRICE_DEVIATION) {
                    LOG_ERROR("ETRADE11",
                              "Pre-check failed for order ID {}: Price "
                              "deviates too much from top of book.",
                              order.getId());
                    return false;
                }
            }
        }

        // Self-match prevention
        if (order.getSide() == common::OrderSide::Buy) {
            if (checkForSelfMatch(order, orderBook.getAskMap())) {
                return false;
            }
        }
        else { // SELL side
            if (checkForSelfMatch(order, orderBook.getBidMap())) {
                return false;
            }
        }

        LOG_INFO("Pre-check passed for order ID {}.", order.getId());
        return true;
    }

    void RiskManager::postTradeUpdate(const common::Trade& trade)
    {
        mTradeRepository->addTrade(trade);
        LOG_INFO("Trade {} added to repository during post-trade update.",
                 trade.getTradeId());
    }

    void RiskManager::postTradeUpdate(const std::vector<common::Trade>& trades)
    {
        for (const auto& trade: trades) { mTradeRepository->addTrade(trade); }
    }
} // namespace trading_core
