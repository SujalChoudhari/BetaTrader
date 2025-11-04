#pragma once
#include "common/Order.h"
#include "common/Trade.h"
#include "data/TradeRepository.h"
#include "trading_core/OrderBook.h"

namespace trading_core {
    class RiskManager {
    public:
        explicit RiskManager(data::TradeRepository* tradeRepository);

        virtual bool preCheck(const common::Order &order, OrderBook& orderBook) ;
        virtual void postTradeUpdate(const common::Trade &trade);
        void postTradeUpdate(const std::vector<common::Trade> &trades);

    private:
        data::TradeRepository* mTradeRepository;
    };
}