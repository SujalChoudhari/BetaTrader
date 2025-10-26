//
// Created by sujal on 26-10-2025.
//

#pragma once
#include "common/Types.h"
#include "SQLiteCpp/Database.h"
#include <memory>
#include <string>

namespace data {
    /**
     * @brief Manages the persistent storage of the last used TradeID.
     * This repository is designed to store and retrieve a single TradeID value
     * from a SQLite database.
     */
    class TradeIDRepository {
    public:
        /**
         * @brief Default constructor. Uses the production database path.
         */
        TradeIDRepository();

        /**
         * @brief Explicit constructor for specifying a database path.
         * @param dbPath Path to the SQLite database file (e.g., ":memory:").
         */
        explicit TradeIDRepository(const std::string& dbPath);

        /**
         * @brief Ensures the necessary database table exists.
         */
        void initDatabase() const;

        /**
         * @brief Retrieves the currently stored TradeID.
         * @return The stored TradeID, or 0 if no ID is stored.
         */
        [[nodiscard]] common::TradeID getCurrentTradeID() const;

        /**
         * @brief Sets and persists the new current TradeID.
         * @param tradeID The TradeID to store.
         */
        void setCurrentTradeID(common::TradeID tradeID) const;

        /**
         * @brief Clears the stored TradeID from the database.
         */
        void truncateTradeID() const;

    private:
        SQLite::Database mDatabase;
    };
}
