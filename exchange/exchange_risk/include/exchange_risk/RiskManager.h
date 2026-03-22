/**
 * @file RiskManager.h
 * @brief Risk management hooks for pre-checks and post-trade updates.
 *
 * The RiskManager provides a place to put pre-order validation and post-trade
 * bookkeeping. Implementations may consult historical trades via the
 * `data::TradeRepository`.
 */

#pragma once
#include "common/Order.h"
#include "common/Trade.h"
#include "data/TradeRepository.h"
#include <exchange_matching/OrderBook.h>

namespace trading_core {
    /**
     * @class RiskManager
     * @brief Performs risk validation and updates after trade execution.
     */
    class RiskManager {
    public:
        virtual ~RiskManager() = default;

        explicit RiskManager(data::TradeRepository* tradeRepository);

        virtual bool preCheck(const common::Order& order, OrderBook& orderBook);
        virtual void postTradeUpdate(const common::Trade& trade);
        void postTradeUpdate(const std::vector<common::Trade>& trades);

    private:
        data::TradeRepository* mTradeRepository;
    };
} // namespace trading_core