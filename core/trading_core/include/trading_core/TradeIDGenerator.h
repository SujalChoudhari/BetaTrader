//
// Created by sujal on 25-10-2025.
//

#pragma once
#include <atomic>

#include "common/Types.h"
#include "data/TradeIDRepository.h"
#include "data/DatabaseWorker.h"

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
         * @param dbWorker A raw pointer to the database worker for persistence.
         */
        explicit TradeIDGenerator(data::DatabaseWorker* dbWorker);

        /**
         * @brief Destructor that saves the final ID state.
         */
        ~TradeIDGenerator();

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
        data::TradeIDRepository repository;      ///< The repository for persisting the trade ID.
    };
}
