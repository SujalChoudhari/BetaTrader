//
// Created by sujal on 29-10-2025.
//

#pragma once
#include "common/Order.h"
#include "common/Trade.h"
#include "data/TradeRepository.h"

namespace trading_core {
    class RiskManager {
    public:
        explicit RiskManager(const data::DatabaseWorkerPtr &dbWorker);

        static bool preCheck(const common::Order &order);

        void postTradeUpdate(const common::Trade &trade);

        void postTradeUpdate(const std::vector<common::Trade> &trades);

    private:
        data::TradeRepository mTradeRepository;
    };
}
