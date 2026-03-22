//
// Created by sujal on 22-10-2025.
//

#pragma once
#include "common/Order.h"
#include "common/Types.h"
#include "trading_core/MarketDataPublisher.h"
#include <deque>
#include <map>
#include <memory>

namespace trading_core {
    /**
     * @class OrderBook
     * @brief Represents the order book for a single financial instrument.
     *
     * The order book maintains two separate books for buy (bid) and sell (ask)
     * orders, organized by price levels. It provides methods for inserting and
     * canceling orders and publishes market data updates.
     */
    class OrderBook {
    public:
        /**
         * @brief A deque of orders at a specific price level.
         */
        using PriceLevel = std::deque<common::Order*>;

        /**
         * @brief A map of prices to price levels for buy orders, sorted in
         * descending order.
         */
        using BidMap = std::map<common::Price, PriceLevel, std::greater<>>;

        /**
         * @brief A map of prices to price levels for sell orders, sorted in
         * ascending order.
         */
        using AskMap = std::map<common::Price, PriceLevel>;

        /**
         * @brief Constructs a new OrderBook.
         * @param symbol The symbol this order book represents.
         * @param publisher A reference to the market data publisher.
         */
        OrderBook(common::Symbol symbol, MarketDataPublisher& publisher);
        virtual ~OrderBook() = default;

        /**
         * @brief Inserts an order into the order book.
         * @param order A raw pointer to the order to be inserted.
         */
        virtual void insertOrder(common::Order* order);

        /**
         * @brief Cancels an order from the order book.
         * @param orderId The ID of the order to be cancelled.
         * @return True if the order was successfully cancelled, false
         * otherwise.
         */
        virtual bool cancelOrder(const common::OrderID& orderId);

        /**
         * @brief Publishes a full snapshot of the current order book for a specific session.
         * @param sessionId The session to send the snapshot to.
         */
        virtual void publishSnapshot(common::SessionID sessionId) const;

        /**
         * @brief Gets a raw pointer to the bid map.
         * @return A raw pointer to the bid map.
         */
        [[nodiscard]] virtual BidMap* getBidMap();

        /**
         * @brief Gets a raw pointer to the ask map.
         * @return A raw pointer to the ask map.
         */
        [[nodiscard]] virtual AskMap* getAskMap();

    private:
        BidMap mBidMap; ///< The map of buy orders.
        AskMap mAskMap; ///< The map of sell orders.
        common::Symbol mSymbol;
        MarketDataPublisher& mPublisher;
    };
} // namespace trading_core
