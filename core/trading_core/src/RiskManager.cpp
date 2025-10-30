#include "trading_core/RiskManager.h"

#include "data/Constant.h"
#include "data/TradeRepository.h"
#include "logging/Logger.h"
#include "trading_core/TradingCoreRunbookDefinations.h"


namespace trading_core {
    RiskManager::RiskManager(const data::DatabaseWorkerPtr &dbWorker) : mTradeRepository(
        data::TradeRepository(dbWorker)) {
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
        }

        LOG_ERROR(errors::ETRADE10, "Pre-check failed for order ID {}. Order details: ID={}, OriginalQty={}, RemainingQty={}, Status={}, Type={}, Price={}",
                  order.getId(), order.getOriginalQuantity(), order.getRemainingQuantity(),
                  static_cast<int>(order.getStatus()), static_cast<int>(order.getOrderType()), order.getPrice());
        return false;
    }

    void RiskManager::postTradeUpdate(const common::Trade &trade) {
        mTradeRepository.addTrade(trade);
        LOG_INFO("Trade {} added to repository during post-trade update.", trade.getTradeId());
    }

    void RiskManager::postTradeUpdate(const std::vector<common::Trade> &trades) {
        for (const auto &trade: trades) {
            mTradeRepository.addTrade(trade);
            LOG_INFO("Trade {} added to repository during batch post-trade update.", trade.getTradeId());
        }
    }
}
