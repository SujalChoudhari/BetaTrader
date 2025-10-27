//
// Created by sujal on 26-10-2025.
//

#pragma once
#include "data/AsyncDatabaseRepository.h"
#include "common/Types.h"

namespace data {
    class TradeIDRepository : public AsyncDatabaseRepository {
    public:
        explicit TradeIDRepository(const std::string &dbPath);

        common::TradeID getCurrentTradeID();

        void setCurrentTradeID(common::TradeID tradeID);

        void truncateTradeID();

        void initDatabase();
    };
}
