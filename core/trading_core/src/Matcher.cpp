#include  "trading_core/Matcher.h"
#include "trading_core/OrderBook.h"
#include <vector>
#include <utility>

#include "logging/Logger.h"
#include "logging/Runbook.h"
#include "trading_core/TradingCoreRunbookDefinations.h"
#include "trading_core/TradeIDGenerator.h"


namespace trading_core {
    Matcher::Matcher(TradeIDGenerator* tradeIdGenerator) : mTradeIdGenerator(tradeIdGenerator) {
    }

    std::vector<common::Trade> Matcher::match(common::Order* incomingOrder, OrderBook &orderBook) {
        std::vector<common::Trade> trades;

        if (!incomingOrder) {
            LOG_ERROR(errors::ETRADE4, "Incoming order is null");
            return trades;
        }

        LOG_INFO("Matching order {} with quantity {}", incomingOrder->getId(), incomingOrder->getRemainingQuantity());

        if (incomingOrder->getSide() == common::OrderSide::Buy) {
            matchTable(incomingOrder, orderBook.getAskMap(), trades);
        } else if (incomingOrder->getSide() == common::OrderSide::Sell) {
            matchTable(incomingOrder, orderBook.getBidMap(), trades);
        }

        LOG_INFO("Order {} matched, {} trades created", incomingOrder->getId(), trades.size());

        return trades;
    }

    template<typename TMap>
    void Matcher::matchTable(common::Order* incomingOrder, TMap* restingMap,
                             std::vector<common::Trade> &trades) {
        auto it = restingMap->begin();

        while (it != restingMap->end() && incomingOrder->getRemainingQuantity() > 0) {
            if (incomingOrder->getOrderType() == common::OrderType::Limit) {
                if (incomingOrder->getSide() == common::OrderSide::Buy) {
                    if (incomingOrder->getPrice() < it->first) break;
                } else { // Sell Limit
                    if (incomingOrder->getPrice() > it->first) break;
                }
            }

            auto &restingLevel = it->second;

            while (!restingLevel.empty() && incomingOrder->getRemainingQuantity() > 0) {
                auto restingOrder = restingLevel.front();

                common::Quantity tradeQuantity = std::min(incomingOrder->getRemainingQuantity(),
                                                          restingOrder->getRemainingQuantity());

                common::Price tradePrice = restingOrder->getPrice();

                common::OrderID buyId = (incomingOrder->getSide() == common::OrderSide::Buy)
                                            ? incomingOrder->getId()
                                            : restingOrder->getId();
                common::OrderID sellId = (incomingOrder->getSide() == common::OrderSide::Buy)
                                             ? restingOrder->getId()
                                             : incomingOrder->getId();

                common::TradeID newId = mTradeIdGenerator->nextId();
                trades.emplace_back(newId, incomingOrder->getSymbol(), buyId, sellId,
                                    tradeQuantity,
                                    tradePrice,
                                    std::chrono::system_clock::now());
                LOG_INFO("New trade created with id {}", newId);

                incomingOrder->setRemainingQuantity(incomingOrder->getRemainingQuantity() - tradeQuantity);
                restingOrder->setRemainingQuantity(restingOrder->getRemainingQuantity() - tradeQuantity);

                incomingOrder->setStatus(common::OrderStatus::PartiallyFilled);

                if (restingOrder->getRemainingQuantity() == 0) {
                    restingOrder->setStatus(common::OrderStatus::Filled);
                    restingLevel.pop_front();
                } else {
                    restingOrder->setStatus(common::OrderStatus::PartiallyFilled);
                }
            }

            if (restingLevel.empty()) {
                it = restingMap->erase(it);
            } else {
                ++it;
            }
        }

        if (incomingOrder->getRemainingQuantity() == 0) {
            incomingOrder->setStatus(common::OrderStatus::Filled);
        } else if (incomingOrder->getOrderType() == common::OrderType::Market) {
            // If a market order is not fully filled, the remainder is cancelled.
            // If it was never filled at all, its original status (e.g., New) should be updated to Cancel.
            if (incomingOrder->getOriginalQuantity() == incomingOrder->getRemainingQuantity()) {
                incomingOrder->setStatus(common::OrderStatus::Cancelled);
            }
            // If it was partially filled, its status is already PartiallyFilled,
            // which is a valid final state for an IOC (Immediate-Or-Cancel) market order.
        }
    }
}