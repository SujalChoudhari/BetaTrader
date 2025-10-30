//
// Created by sujal on 22-10-2025.
//

#pragma once
#include <deque>
#include <map>
#include <memory>

#include "common/Order.h"
#include "common/Types.h"

namespace trading_core {
    /**
     * @class OrderBook
     * @brief Represents the order book for a single financial instrument.
     *
     * The order book maintains two separate books for buy (bid) and sell (ask) orders,
     * organized by price levels. It provides methods for inserting and canceling orders.
     */
    class OrderBook {
        /**
         * @brief A deque of orders at a specific price level.
         */
        using PriceLevel = std::deque<std::shared_ptr<common::Order>>;

        /**
         * @brief A map of prices to price levels for buy orders, sorted in descending order.
         */
        using BidMap = std::map<common::Price, PriceLevel, std::greater<> >;

        /**
         * @brief A map of prices to price levels for sell orders, sorted in ascending order.
         */
        using AskMap = std::map<common::Price, PriceLevel>;

    public:
        /**
         * @brief Inserts an order into the order book.
         * @param order A shared pointer to the order to be inserted.
         */
        void insertOrder(std::shared_ptr<common::Order> order);

        /**
         * @brief Cancels an order from the order book.
         * @param orderId The ID of the order to be cancelled.
         * @return True if the order was successfully cancelled, false otherwise.
         */
        bool cancelOrder(const common::OrderID &orderId);

        /**
         * @brief Gets a shared pointer to the bid map.
         * @return A shared pointer to the bid map.
         */
        [[nodiscard]] std::shared_ptr<BidMap> getBidMap();

        /**
         * @brief Gets a shared pointer to the ask map.
         * @return A shared pointer to the ask map.
         */
        [[nodiscard]] std::shared_ptr<AskMap> getAskMap();

    private:
        BidMap mBidMap; ///< The map of buy orders.
        AskMap mAskMap;   ///< The map of sell orders.
    };
}
