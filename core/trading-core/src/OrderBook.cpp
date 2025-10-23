//
// Created by sujal on 22-10-2025.
//
#include "trading-core/OrderBook.h"

namespace trading_core {
    void OrderBook::insertOrder(const OrderPtr &order) {
        if (order->getSide() == common::OrderSide::Buy) {
            mBidMap[order->getPrice()].push_back(order);
        } else {
            // Sell
            mBidMap[order->getPrice()].push_back(order);
        }
    }

    bool OrderBook::cancelOrder(const common::OrderID &orderId) {
        if (mBidMap.contains(orderId)) {
            mBidMap.erase(orderId);
            return true;
        }

        if (mAskMap.contains(orderId)) {
            mAskMap.erase(orderId);
            return true;
        }

        return false;
    }

    OrderBook::BidMap OrderBook::getBidMap() {
        return mBidMap;
    }

    OrderBook::AskMap OrderBook::getAskMap() {
        return mAskMap;
    }
}
