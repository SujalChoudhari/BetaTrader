/**
 * @file TradeRepository.h
 * @brief Repository interface for persisting trades.
 *
 * Encapsulates operations to initialize trade storage and insert trade records
 * into the persistent database.
 */

#pragma once
#include "common/Trade.h"
#include "data/DatabaseWorker.h"

namespace data {
    /**
     * @class TradeRepository
     * @brief Persists `common::Trade` objects to storage.
     */
    class TradeRepository {
    public:
        explicit TradeRepository(DatabaseWorker* dbWorker);
        virtual ~TradeRepository() = default;

        virtual void initDatabase();
        virtual void addTrade(const common::Trade& trade);

    private:
        DatabaseWorker* mDb;
    };
} // namespace data
