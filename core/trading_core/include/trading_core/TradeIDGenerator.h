//
// Created by sujal on 25-10-2025.
//

#pragma once
#include <mutex>
#include <atomic>

#include "common/Types.h"
#include "data/TradeIDRepository.h"

namespace trading_core {
    /**
     * @class TradeIDGenerator
     * @brief A thread-safe generator for unique trade IDs.
     *
     * This class provides a thread-safe interface for generating unique, sequential trade IDs.
     * It also interacts with a TradeIDRepository to persist the current ID and load it upon initialization.
     */
    class TradeIDGenerator {
    public:
        /**
         * @brief Constructs a new TradeIDGenerator object.
         * @param dbWorker A shared pointer to the database worker for persistence.
         */
        explicit TradeIDGenerator(const data::DatabaseWorkerPtr &dbWorker);

        /**
         * @brief Gets the current trade ID.
         * @return The current trade ID.
         */
        common::TradeID getId();

        /**
         * @brief Gets the next available trade ID.
         * @return The next available trade ID.
         */
        common::TradeID nextId();

        /**
         * @brief Saves the current state of the generator to the repository.
         */
        void saveState();

        /**
         * @brief Loads the state of the generator from the repository.
         */
        void loadState();

    private:
        std::atomic<common::TradeID> mCurrentId; ///< The current trade ID.
        std::mutex mMutex;                       ///< A mutex for thread-safe state management.
        data::TradeIDRepository repository;      ///< The repository for persisting the trade ID.
    };
}
