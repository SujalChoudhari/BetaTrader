#include <exchange_matching/OrderBook.h>
#include "logging/Logger.h"
#include <algorithm>
#include <chrono>

namespace trading_core {

    OrderBook::OrderBook(common::Symbol symbol, MarketDataPublisher& publisher)
        : mSymbol(symbol), mPublisher(publisher)
    {}

    void OrderBook::insertOrder(common::Order* order)
    {
        if (order->getSide() == common::OrderSide::Buy) {
            mBidMap[order->getPrice()].push_back(order);
        }
        else {
            mAskMap[order->getPrice()].push_back(order);
        }

        fix::MarketDataIncrementalRefresh refresh;
        refresh.symbol = mSymbol;
        
        fix::MarketDataIncrementalEntry entry;
        entry.updateAction = fix::MDUpdateAction::New;
        entry.entryType = (order->getSide() == common::OrderSide::Buy)
                                  ? fix::MDEntryType::Bid
                                  : fix::MDEntryType::Offer;
        entry.price = order->getPrice();
        entry.size = order->getRemainingQuantity();
        entry.entryTime = order->getTimestamp();
        entry.entryPosition = 0;
        
        refresh.entries.push_back(entry);
        mPublisher.publishIncremental(refresh);
    }

    bool OrderBook::cancelOrder(const common::OrderID& orderId)
    {
        common::Order* cancelledOrder = nullptr;

        for (auto bidIt = mBidMap.begin(); bidIt != mBidMap.end();) {
            auto& orders = bidIt->second;
            auto orderIt
                    = std::ranges::find_if(orders, [&](const common::Order* o) {
                          return o->getClientOrderId() == orderId;
                      });
            if (orderIt != orders.end()) {
                cancelledOrder = *orderIt;
                orders.erase(orderIt);
                if (orders.empty()) {
                    bidIt = mBidMap.erase(bidIt);
                } else {
                    ++bidIt;
                }
                break;
            } else {
                ++bidIt;
            }
        }

        if (!cancelledOrder) {
            for (auto askIt = mAskMap.begin(); askIt != mAskMap.end();) {
                auto& orders = askIt->second;
                auto orderIt = std::ranges::find_if(
                        orders, [&](const common::Order* o) {
                            return o->getClientOrderId() == orderId;
                        });
                if (orderIt != orders.end()) {
                    cancelledOrder = *orderIt;
                    orders.erase(orderIt);
                    if (orders.empty()) {
                        askIt = mAskMap.erase(askIt);
                    } else {
                        ++askIt;
                    }
                    break;
                } else {
                    ++askIt;
                }
            }
        }

        if (cancelledOrder) {
            fix::MarketDataIncrementalRefresh refresh;
            refresh.symbol = mSymbol;
            
            fix::MarketDataIncrementalEntry entry;
            entry.updateAction = fix::MDUpdateAction::Delete;
            entry.entryType
                    = (cancelledOrder->getSide() == common::OrderSide::Buy)
                              ? fix::MDEntryType::Bid
                              : fix::MDEntryType::Offer;
            entry.price = cancelledOrder->getPrice();
            entry.size = cancelledOrder->getRemainingQuantity();
            entry.entryTime = std::chrono::system_clock::now();
            entry.entryPosition = 0;
            
            refresh.entries.push_back(entry);
            mPublisher.publishIncremental(refresh);
            return true;
        }

        return false;
    }

    void OrderBook::publishSnapshot(common::SessionID sessionId) const
    {
        fix::MarketDataSnapshotFullRefresh snapshot;
        snapshot.targetSessionID = sessionId;
        snapshot.symbol = mSymbol;

        uint32_t position = 1;
        for (const auto& [price, orders]: mBidMap) {
            if (orders.empty()) continue;
            
            fix::MarketDataEntry entry;
            entry.entryType = fix::MDEntryType::Bid;
            entry.price = price;
            
            common::Quantity totalQty = 0;
            for (const auto& order: orders) {
                totalQty += order->getRemainingQuantity();
            }
            entry.size = totalQty;
            
            entry.entryTime = orders.front()->getTimestamp(); 
            entry.entryPosition = position++;
            snapshot.entries.push_back(entry);
        }

        position = 1;
        for (const auto& [price, orders]: mAskMap) {
            if (orders.empty()) continue;
            
            fix::MarketDataEntry entry;
            entry.entryType = fix::MDEntryType::Offer;
            entry.price = price;
            
            common::Quantity totalQty = 0;
            for (const auto& order: orders) {
                totalQty += order->getRemainingQuantity();
            }
            entry.size = totalQty;

            entry.entryTime = orders.front()->getTimestamp();
            entry.entryPosition = position++;
            snapshot.entries.push_back(entry);
        }
        mPublisher.publishSnapshot(snapshot);
    }

    OrderBook::BidMap* OrderBook::getBidMap()
    {
        return &mBidMap;
    }

    OrderBook::AskMap* OrderBook::getAskMap()
    {
        return &mAskMap;
    }
} // namespace trading_core
