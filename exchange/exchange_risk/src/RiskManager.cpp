#include <exchange_risk/RiskManager.h>
#include "logging/Logger.h"

namespace trading_core {

    constexpr double MAX_PRICE_DEVIATION = 0.10; // 10% deviation

    // Check every price level that the incoming order could cross, not only the
    // best level. The risk gate runs before matching, so rejecting the incoming
    // order here prevents the matcher from reaching a same-owner order deeper
    // in the book.
    template<typename TMap>
    bool checkForSelfMatch(const common::Order& order, const TMap* oppositeMap)
    {
        for (const auto& [price, priceLevel]: *oppositeMap) {
            if (order.getOrderType() == common::OrderType::Limit) {
                const bool crosses = order.getSide() == common::OrderSide::Buy
                                             ? order.getPrice() >= price
                                             : order.getPrice() <= price;
                if (!crosses) { break; }
            }

            for (const auto* restingOrder: priceLevel) {
                if (restingOrder->getSenderCompID()
                    == order.getSenderCompID()) {
                    LOG_ERROR("ETRADE12",
                              "Pre-check failed for order ID {}: "
                              "Self-match detected with {} order.",
                              order.getClientOrderId(),
                              order.getOrderType() == common::OrderType::Market
                                      ? "market"
                                      : "limit");
                    return true;
                }
            }
        }
        return false; // No self-match at any crossing price level
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
                      order.getClientOrderId());
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
                              order.getClientOrderId());
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

        LOG_INFO("Pre-check passed for order ID {}.", order.getClientOrderId());
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
