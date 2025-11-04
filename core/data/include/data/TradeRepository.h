#pragma once
#include "data/DatabaseWorker.h"
#include "common/Trade.h"

namespace data {
    class TradeRepository {
    public:
        explicit TradeRepository(DatabaseWorker* dbWorker);
        virtual ~TradeRepository() = default;

        virtual void initDatabase();
        virtual void addTrade(const common::Trade &trade);

    private:
        DatabaseWorker* mDb;
    };
}
