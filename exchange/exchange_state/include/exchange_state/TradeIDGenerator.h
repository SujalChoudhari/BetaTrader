//
// Created by sujal on 25-10-2025.
//

#pragma once
#include "common/Types.h"
#include "data/TradeIDRepository.h"
#include <atomic>

namespace trading_core {
    /**
     * @class TradeIDGenerator
     * @brief A thread-safe, database-aware generator for unique trade IDs.
     */
    class TradeIDGenerator {
    public:
        explicit TradeIDGenerator(data::TradeIDRepository* tradeIdRepo);
        ~TradeIDGenerator();

        /**
         * @brief Gets the next available trade ID.
         * @return The next available trade ID.
         */
        virtual common::TradeID nextId();

    private:
        void loadInitialState();
        void saveState();

        std::atomic<common::TradeID> mCurrentId;
        data::TradeIDRepository* mTradeIDRepo;
    };
} // namespace trading_core
