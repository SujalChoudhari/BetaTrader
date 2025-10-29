//
// Created by sujal on 26-10-2025.
//

#pragma once
#include "common/Types.h"
#include "data/DatabaseWorker.h"

namespace data {

    class TradeIDRepository {
    public:
        explicit TradeIDRepository(DatabaseWorker& dbWorker);
        TradeIDRepository(std::shared_ptr<data::DatabaseWorker> dbWorker);

        void initDatabase();
        common::TradeID getCurrentTradeID();
        void setCurrentTradeID(common::TradeID tradeID);
        void truncateTradeID();

    private:
        DatabaseWorker& mDb;
    };

}