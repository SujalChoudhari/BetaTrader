//
// Created by sujal on 27-10-2025.
//

#pragma once
#include "common/Trade.h"
#include "SQLiteCpp/Database.h"

namespace data {
    class TradeRepository {
    public:
        TradeRepository();

        void initDatabase() const;

        void addTrade(const common::Trade &trade) const;

    private:
        SQLite::Database mDatabase;
    };
} // data
