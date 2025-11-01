//
// Created by sujal on 27-10-2025.
//

#pragma once
#include "data/DatabaseWorker.h"
#include "common/Trade.h"

namespace data {
    class TradeRepository {
    public:
        explicit TradeRepository(DatabaseWorker* dbWorker);

        void initDatabase();

        void addTrade(const common::Trade &trade);

    private:
        DatabaseWorker* mDb;
    };
}
