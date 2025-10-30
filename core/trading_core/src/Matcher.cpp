//
// Created by sujal on 25-10-2025.
//


#include  "trading_core/Matcher.h"
#include "trading_core/OrderBook.h"
#include <vector>

#include "logging/Logger.h"
#include "logging/Runbook.h"
#include "trading_core/TradingCoreRunbookDefinations.h"
#include "trading_core/TradeIDGenerator.h"


namespace trading_core {
    Matcher::Matcher(const data::DatabaseWorkerPtr &dbWorker) : mTradeIdGenerator(TradeIDGenerator(dbWorker)) {
    }

    std::vector<common::Trade> Matcher::match(common::Order &incomingOrder, OrderBook &orderBook) {
        std::vector<common::Trade> trades;

        if (incomingOrder.getSide() == common::OrderSide::Buy) {
            matchTable(incomingOrder, orderBook.getAskMap(), trades);
        } else if (incomingOrder.getSide() == common::OrderSide::Sell) {
            matchTable(incomingOrder, orderBook.getBidMap(), trades);
        }

        return trades;
    }

    template<typename TMap>
    void Matcher::matchTable(common::Order &incomingOrder, std::shared_ptr<TMap> restingMap,
                             std::vector<common::Trade> &trades) {
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

            std::deque<common::Order *> &restingLevel = it->second;


            // apply and update the book
            while (!restingLevel.empty() && incomingOrder.getRemainingQuantity() > 0) {
                common::Order *restingOrder = restingLevel.front();

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

                trades.emplace_back(mTradeIdGenerator.nextId(), incomingOrder.getSymbol(), buyId, sellId, tradeQuantity,
                                    tradePrice,
                                    std::chrono::system_clock::now());

                incomingOrder.setRemainingQuantity(incomingOrder.getRemainingQuantity() - tradeQuantity);
                restingOrder->setRemainingQuantity(restingOrder->getRemainingQuantity() - tradeQuantity);

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
