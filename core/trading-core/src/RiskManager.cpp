//
// Created by sujal on 29-10-2025.
//

#include "trading-core/RiskManager.h"

#include "data/Constant.h"
#include "data/TradeRepository.h"


namespace trading_core {
    RiskManager::RiskManager()
        : mTradeRepository(data::databasePath) {
    }

    bool RiskManager::preCheck(const common::Order &order) {
        if (
            order.getId() > 0
            && order.getOriginalQuantity() > 0
            && order.getRemainingQuantity() == order.getOriginalQuantity()
            && order.getStatus() == common::OrderStatus::New
            && (order.getOrderType() == common::OrderType::Market || order.getPrice() > 0)
        ) {
            return true;
        }

        // TODO: handle fat finger errors

        return false;
    }

    void RiskManager::postTradeUpdate(const common::Trade &trade) {
        mTradeRepository.addTrade(trade);
    }

    void RiskManager::postTradeUpdate(const std::vector<common::Trade> &trades) {
        for (const auto &trade: trades) {
            mTradeRepository.addTrade(trade);
        }
    }
}
