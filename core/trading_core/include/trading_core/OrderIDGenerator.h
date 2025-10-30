//
// Created by sujal on 25-10-2025.
//

#pragma once
#include <mutex>
#include <atomic>

#include "common/Types.h"

namespace trading_core {
    /**
     * @class OrderIDGenerator
     * @brief A thread-safe generator for unique order IDs.
     *
     * This class provides a static interface for generating unique, sequential order IDs.
     * It is designed to be thread-safe, allowing it to be used by multiple threads
     * simultaneously.
     */
    class OrderIDGenerator {
        static std::atomic<common::OrderID> currentId; ///< The current order ID.
        static std::mutex mutex;                       ///< A mutex for thread-safe state management.

    public:
        /**
         * @brief Gets the current order ID.
         * @return The current order ID.
         */
        static common::OrderID getId();

        /**
         * @brief Gets the next available order ID.
         * @return The next available order ID.
         */
        static common::OrderID nextId();

        /**
         * @brief Saves the current state of the generator.
         * @note This method is not yet implemented.
         */
        static void saveState();

        /**
         * @brief Loads the state of the generator.
         * @note This method is not yet implemented.
         */
        static void loadState();
    };

}
