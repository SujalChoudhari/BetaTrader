//
// Created by sujal on 21-10-2025.
//

#pragma once

#include "Types.h"
#include "Time.h"

namespace common {
    /**
     * Representation of fill actions of orders
     */
    class Trade {
    public:
        Trade(TradeID tradeId,
              OrderSymbol symbol,
              OrderID buyOrderId,
              OrderID sellOrderId,
              const Quantity qty,
              const Price price,
              const Timestamp ts)
            : mTradeId(std::move(tradeId)),
              mOrderSymbol(std::move(symbol)),
              mBuyOrderId(std::move(buyOrderId)),
              mSellOrderId(std::move(sellOrderId)),
              mQuantity(qty),
              mPrice(price),
              mTimestamp(ts) {
        }

    public:
        [[nodiscard]] const TradeID &getTradeId() const { return mTradeId; }
        [[nodiscard]] const OrderSymbol &getOrderSymbol() const { return mOrderSymbol; }
        [[nodiscard]] const OrderID &getBuyOrderId() const { return mBuyOrderId; }
        [[nodiscard]] const OrderID &getSellOrderId() const { return mSellOrderId; }
        [[nodiscard]] Quantity getQty() const { return mQuantity; }
        [[nodiscard]] Price getPrice() const { return mPrice; }
        [[nodiscard]] Timestamp getTimestamp() const { return mTimestamp; }

    private:
        TradeID mTradeId;
        OrderSymbol mOrderSymbol;
        OrderID mBuyOrderId;
        OrderID mSellOrderId;
        Quantity mQuantity;
        Price mPrice;
        Timestamp mTimestamp;
    };
}
