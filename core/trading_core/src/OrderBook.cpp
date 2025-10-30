//
// Created by sujal on 22-10-2025.
//
#include "trading_core/OrderBook.h"

namespace trading_core {
    void OrderBook::insertOrder(std::shared_ptr<common::Order> order) {
        if (order->getSide() == common::OrderSide::Buy) {
            mBidMap[order->getPrice()].push_back(order);
        } else {
            mAskMap[order->getPrice()].push_back(order);
        }
    }

    bool OrderBook::cancelOrder(const common::OrderID &orderId) {
        for (auto bidIt = mBidMap.begin(); bidIt != mBidMap.end();) {
            auto &orders = bidIt->second;
            auto orderIt = std::ranges::find_if(orders,
                                                [&](const std::shared_ptr<common::Order> &o) {
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
            auto &orders = askIt->second;
            auto orderIt = std::ranges::find_if(orders,
                                                [&](const std::shared_ptr<common::Order> &o) {
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


    std::shared_ptr<OrderBook::BidMap> OrderBook::getBidMap() {
        return std::make_shared<BidMap>(mBidMap);
    }

    std::shared_ptr<OrderBook::AskMap> OrderBook::getAskMap() {
        return std::make_shared<AskMap>(mAskMap);
    }
}
