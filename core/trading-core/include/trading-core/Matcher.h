//
// Created by sujal on 23-10-2025.
//

#pragma once
#include <vector>

#include "OrderBook.h"
#include "TradeIDGenerator.h"
#include "common/Trade.h"
#include "data/DatabaseWorker.h"
#include "data/TradeRepository.h"

namespace trading_core {
    class Matcher {
    public:
        Matcher(const data::DatabaseWorkerPtr &dbWorker);

        std::vector<common::Trade> match(common::Order &incomingOrder, OrderBook &orderBook);

    private:
        template<typename TMap>
        void matchTable(common::Order &incomingOrder, std::shared_ptr<TMap> restingMap,
                        std::vector<common::Trade> &trades);

    private:
        TradeIDGenerator mTradeIdGenerator;
    };
}
