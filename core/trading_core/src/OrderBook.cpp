#include "trading_core/OrderBook.h"

namespace trading_core {
    void OrderBook::insertOrder(common::Order* order)
    {
        if (order->getSide() == common::OrderSide::Buy) {
            mBidMap[order->getPrice()].push_back(order);
        }
        else {
            mAskMap[order->getPrice()].push_back(order);
        }
    }

    bool OrderBook::cancelOrder(const common::OrderID& orderId)
    {
        for (auto bidIt = mBidMap.begin(); bidIt != mBidMap.end();) {
            auto& orders = bidIt->second;
            auto orderIt
                    = std::ranges::find_if(orders, [&](const common::Order* o) {
                          return o->getId() == orderId;
                      });
            if (orderIt != orders.end()) {
                orders.erase(orderIt);
                if (orders.empty())
                    bidIt = mBidMap.erase(bidIt);
                else
                    ++bidIt;
                return true;
            }
            ++bidIt;
        }

        for (auto askIt = mAskMap.begin(); askIt != mAskMap.end();) {
            auto& orders = askIt->second;
            auto orderIt
                    = std::ranges::find_if(orders, [&](const common::Order* o) {
                          return o->getId() == orderId;
                      });
            if (orderIt != orders.end()) {
                orders.erase(orderIt);
                if (orders.empty())
                    askIt = mAskMap.erase(askIt);
                else
                    ++askIt;
                return true;
            }
            ++askIt;
        }

        return false;
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
