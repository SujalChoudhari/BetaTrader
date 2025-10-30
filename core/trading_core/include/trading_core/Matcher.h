//
// Created by sujal on 23-10-2025.
//

#pragma once
#include <vector>
#include <memory>

#include "OrderBook.h"
#include "TradeIDGenerator.h"
#include "common/Trade.h"
#include "data/DatabaseWorker.h"
#include "data/TradeRepository.h"

namespace trading_core {
    /**
     * @class Matcher
     * @brief The matching engine that pairs buy and sell orders.
     *
     * This class is responsible for matching incoming orders against the existing orders
     * in the order book. It generates trades when a match is found.
     */
    class Matcher {
    public:
        /**
         * @brief Constructs a new Matcher object.
         * @param tradeIdGenerator A shared pointer to the trade ID generator.
         */
        Matcher(std::shared_ptr<TradeIDGenerator> tradeIdGenerator);

        /**
         * @brief Matches an incoming order against the order book.
         * @param incomingOrder The order to be matched.
         * @param orderBook The order book to match against.
         * @return A vector of trades that were generated.
         */
        std::vector<common::Trade> match(std::shared_ptr<common::Order> incomingOrder, OrderBook &orderBook);

    private:
        /**
         * @brief A template function to match an incoming order against a map of resting orders.
         * @tparam TMap The type of the map of resting orders (BidMap or AskMap).
         * @param incomingOrder The order to be matched.
         * @param restingMap A shared pointer to the map of resting orders.
         * @param trades A reference to a vector of trades to which any generated trades will be added.
         */
        template<typename TMap>
        void matchTable(std::shared_ptr<common::Order> incomingOrder, std::shared_ptr<TMap> restingMap,
                        std::vector<common::Trade> &trades);

    private:
        std::shared_ptr<TradeIDGenerator> mTradeIdGenerator; ///< A generator for unique trade IDs.
    };
}
