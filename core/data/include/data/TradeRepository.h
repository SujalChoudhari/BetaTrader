//
// Created by sujal on 27-10-2025.
//

#pragma once
#include "AsyncDatabaseRepository.h"
#include "common/Trade.h"
#include "SQLiteCpp/Database.h"

namespace data {
    class TradeRepository : public AsyncDatabaseRepository {
    public:
        explicit TradeRepository(const std::string &dbPath);

        void addTrade(const common::Trade &trade);

    private:
        void initDatabase();
    };
} // data
