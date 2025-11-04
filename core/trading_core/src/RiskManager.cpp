#include "trading_core/RiskManager.h"
#include "logging/Logger.h"

namespace trading_core {
    RiskManager::RiskManager(data::TradeRepository* tradeRepository) : mTradeRepository(tradeRepository) {
        LOG_INFO("RiskManager initialized.");
    }

    bool RiskManager::preCheck(const common::Order &order) {
        if (
            order.getId() > 0
            && order.getOriginalQuantity() > 0
            && order.getRemainingQuantity() == order.getOriginalQuantity()
            && order.getStatus() == common::OrderStatus::New
            && (order.getOrderType() == common::OrderType::Market || order.getPrice() > 0)
        ) {
            LOG_INFO("Pre-check passed for order ID {}.", order.getId());
            return true;
        } else {
            LOG_ERROR("ETRADE10", "Pre-check failed for order ID {}.", order.getId());
            return false;
        }
    }

    void RiskManager::postTradeUpdate(const common::Trade &trade) {
        mTradeRepository->addTrade(trade);
        LOG_INFO("Trade {} added to repository during post-trade update.", trade.getTradeId());
    }

    void RiskManager::postTradeUpdate(const std::vector<common::Trade> &trades) {
        for (const auto &trade: trades) {
            mTradeRepository->addTrade(trade);
        }
    }
}
