//
// Created by sujal on 21-10-2025.
//

#pragma once

#include "common/Types.h"
#include "common/Time.h"

namespace trading_core {
    /**
     * Representation of fill actions of orders
     */
    class Trade {
    public:
        Trade(common::TradeID tradeId,
              common::OrderID buyOrderId,
              common::OrderID sellOrderId,
              const common::Quantity qty,
              const common::Price price,
              const common::Timestamp ts)
            : mTradeId(std::move(tradeId)),
              mBuyOrderId(std::move(buyOrderId)),
              mSellOrderId(std::move(sellOrderId)),
              mQuantity(qty),
              mPrice(price),
              mTimestamp(ts) {
        }

    public:
        [[nodiscard]] const common::TradeID &getTradeId() const { return mTradeId; }
        [[nodiscard]] const common::OrderID &getBuyOrderId() const { return mBuyOrderId; }
        [[nodiscard]] const common::OrderID &getSellOrderId() const { return mSellOrderId; }
        [[nodiscard]] common::Quantity getQty() const { return mQuantity; }
        [[nodiscard]] common::Price getPrice() const { return mPrice; }
        [[nodiscard]] common::Timestamp getTimestamp() const { return mTimestamp; }

    private:
        common::TradeID mTradeId;
        common::OrderID mBuyOrderId;
        common::OrderID mSellOrderId;
        common::Quantity mQuantity;
        common::Price mPrice;
        common::Timestamp mTimestamp;
    };
}
