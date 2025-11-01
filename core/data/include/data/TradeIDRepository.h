//
// Created by sujal on 26-10-2025.
//

#pragma once
#include "common/Types.h"
#include "data/DatabaseWorker.h"
#include <functional>

namespace data {

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