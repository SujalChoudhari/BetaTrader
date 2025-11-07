/**
 * @file TradeIDRepository.h
 * @brief Repository to persist and retrieve the current trade ID.
 *
 * The TradeIDRepository provides the persistent backing for the
 * TradeIDGenerator so the generator can restore and save the last
 * generated trade id across process restarts.
 */

#pragma once
#include "common/Types.h"
#include "data/DatabaseWorker.h"
#include <functional>

namespace data {

    /**
     * @class TradeIDRepository
     * @brief Persistent storage for trade id value.
     */
    class TradeIDRepository {
    public:
        explicit TradeIDRepository(DatabaseWorker* dbWorker);

        void initDatabase();
        void getCurrentTradeID(std::function<void(common::TradeID)> callback);
        void setCurrentTradeID(common::TradeID tradeID);
        void truncateTradeID();

    private:
        DatabaseWorker* mDb;
    };

}