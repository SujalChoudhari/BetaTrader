//
// Created by sujal on 25-10-2025.
//

#pragma once
#include "common/Types.h"
#include "data/DatabaseWorker.h"
#include <atomic>

namespace trading_core {
    /**
     * @class OrderIDGenerator
     * @brief A thread-safe, database-aware generator for unique order IDs.
     */
    class OrderIDGenerator {
    public:
        explicit OrderIDGenerator(data::DatabaseWorker* dbWorker);

        /**
         * @brief Gets the next available order ID.
         * @return The next available order ID.
         */
        virtual common::OrderID nextId();

    private:
        void loadInitialState();

        std::atomic<common::OrderID> mCurrentId;
        data::DatabaseWorker* mDatabaseWorker;
    };

} // namespace trading_core
