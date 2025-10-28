//
// Created by sujal on 25-10-2025.
//


#include  "trading-core/Matcher.h"
#include "trading-core/OrderBook.h"
#include <vector>

#include "logging/Logger.h"
#include "logging/Runbook.h"
#include "trading-core/RunbookDefinations.h"
#include "trading-core/TradeIDGenerator.h"


namespace trading_core {
    std::vector<common::Trade> Matcher::match(common::Order &incomingOrder, OrderBook &orderBook) const {
        std::vector<common::Trade> trades;

        LOG_TRACE("Attempting to match {} of side {}", incomingOrder.getId(), incomingOrder.getSide());
        LOG_ERROR(errors::E1001, "Attempting to match {} of side", incomingOrder.getId());
        if (incomingOrder.getSide() == common::OrderSide::Buy) {
            matchTable(incomingOrder, orderBook.getAskMap(), trades);
        } else if (incomingOrder.getSide() == common::OrderSide::Sell) {
            matchTable(incomingOrder, orderBook.getBidMap(), trades);
        }

        LOG_TRACE("Match complete with {} trades", trades.size());

        return trades;
    }

    template<typename TMap>
    void Matcher::matchTable(common::Order &incomingOrder, std::shared_ptr<TMap> restingMap,
                             std::vector<common::Trade> &trades) const {
        auto it = restingMap->begin();

        while (it != restingMap->end() && incomingOrder.getRemainingQuantity() > 0) {
            if (incomingOrder.getOrderType() == common::OrderType::Market) {
                // Market order but at any price
            } else if (incomingOrder.getSide() == common::OrderSide::Buy) {
                // Can buy below the limit
                if (incomingOrder.getPrice() < it->first) break;
            } else {
                // Can sell above the limit
                if (incomingOrder.getPrice() > it->first) break;
            }

            std::deque<common::OrderPtr> &restingLevel = it->second;


            // apply and update the book
            while (!restingLevel.empty() && incomingOrder.getRemainingQuantity() > 0) {
                common::Order *restingOrder = restingLevel.front().get();

                common::Quantity tradeQuantity = std::min(incomingOrder.getRemainingQuantity(),
                                                          restingOrder->getRemainingQuantity());

                common::Price tradePrice = restingOrder->getPrice();

                // create order
                common::OrderID buyId = (incomingOrder.getSide() == common::OrderSide::Buy)
                                            ? incomingOrder.getId()
                                            : restingOrder->getId();
                common::OrderID sellId = (incomingOrder.getSide() == common::OrderSide::Buy)
                                             ? restingOrder->getId()
                                             : incomingOrder.getId();

                trades.emplace_back(TradeIDGenerator::nextId(), buyId, sellId, tradeQuantity, tradePrice,
                                    std::chrono::steady_clock::now());

                incomingOrder.setRemainingQty(incomingOrder.getRemainingQuantity() - tradeQuantity);
                restingOrder->setRemainingQty(restingOrder->getRemainingQuantity() - tradeQuantity);

                incomingOrder.setStatus(common::OrderStatus::PartiallyFilled);

                if (restingOrder->getRemainingQuantity() == 0) {
                    restingOrder->setStatus(common::OrderStatus::Filled);
                    restingLevel.pop_front();
                } else {
                    restingOrder->setStatus(common::OrderStatus::PartiallyFilled);
                }
            }

            // empty price level
            if (restingLevel.empty()) {
                it = restingMap->erase(it);
            } else {
            }
        }

        if (incomingOrder.getRemainingQuantity() == 0) {
            incomingOrder.setStatus(common::OrderStatus::Filled);
        }
    }
}
