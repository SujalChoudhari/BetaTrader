//
// Created by sujal on 25-10-2025.
//

#pragma once
#include "common/Types.h"
#include "data/DatabaseWorker.h"
#include <atomic>

namespace trading_core {
    /**
     * @class TradeIDGenerator
     * @brief A thread-safe, database-aware generator for unique trade IDs.
     */
    class TradeIDGenerator {
    public:
        explicit TradeIDGenerator(data::DatabaseWorker* dbWorker);

        /**
         * @brief Gets the next available trade ID.
         * @return The next available trade ID.
         */
        virtual common::TradeID nextId();

    private:
        void loadInitialState();

        std::atomic<common::TradeID> mCurrentId;
        data::DatabaseWorker* mDatabaseWorker;
    };
} // namespace trading_core
