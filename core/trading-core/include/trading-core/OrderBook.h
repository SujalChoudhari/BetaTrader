//
// Created by sujal on 22-10-2025.
//

#pragma once
#include <deque>
#include <map>

#include "Order.h"
#include "common/Types.h"

namespace trading_core {
    class OrderBook {
        // A Level with all the orders arranged in queue based on their price
        // Same tier prices are arranged in same level
        using PriceLevel = std::deque<OrderPtr>;

        // Map of the price tier and the queue of the orders of the Buyers
        using BidMap = std::map<common::Price, PriceLevel, std::greater<common::Price> >;

        // Map of the price tier and the queue of the orders of the Sellers
        using AskMap = std::map<common::Price, PriceLevel>;

        /**
         *
         * @param order Order to insert in the Book
         */
        void insertOrder(const OrderPtr &order);

        /**
         *
         * @param orderId OrderId to cancel
         * @return true if successfully deleted, else false
         */
        bool cancelOrder(const common::OrderID &orderId);

    private:
        [[nodiscard]] BidMap getBidMap();

        [[nodiscard]] AskMap getAskMap();

    private:
        BidMap mBidMap;
        AskMap mAskMap;
    };
}
