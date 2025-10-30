//
// Created by sujal on 21-10-2025.
//

#pragma once

#include "Types.h"
#include "Time.h"

namespace common {
    /**
     * @class Trade
     * @brief Represents a single trade execution in the system.
     *
     * This class is a data model that encapsulates all the properties of a trade,
     * including its ID, symbol, buy/sell order IDs, quantity, price, and timestamp.
     */
    class Trade {
    public:
        /**
         * @brief Constructs a new Trade object.
         * @param tradeId The unique identifier for the trade.
         * @param symbol The financial instrument symbol.
         * @param buyOrderId The ID of the buy order involved in the trade.
         * @param sellOrderId The ID of the sell order involved in the trade.
         * @param qty The quantity of the trade.
         * @param price The price at which the trade was executed.
         * @param ts The timestamp of when the trade occurred.
         */
        Trade(TradeID tradeId,
              Instrument symbol,
              OrderID buyOrderId,
              OrderID sellOrderId,
              const Quantity qty,
              const Price price,
              const Timestamp ts)
            : mTradeId(tradeId),
              mOrderSymbol(symbol),
              mBuyOrderId(buyOrderId),
              mSellOrderId(sellOrderId),
              mQuantity(qty),
              mPrice(price),
              mTimestamp(ts) {
        }

    public:
        /** @brief Gets the trade's unique identifier. */
        [[nodiscard]] const TradeID &getTradeId() const { return mTradeId; }
        /** @brief Gets the financial instrument symbol. */
        [[nodiscard]] const Instrument &getOrderSymbol() const { return mOrderSymbol; }
        /** @brief Gets the buy order's unique identifier. */
        [[nodiscard]] const OrderID &getBuyOrderId() const { return mBuyOrderId; }
        /** @brief Gets the sell order's unique identifier. */
        [[nodiscard]] const OrderID &getSellOrderId() const { return mSellOrderId; }
        /** @brief Gets the trade quantity. */
        [[nodiscard]] Quantity getQuantity() const { return mQuantity; }
        /** @brief Gets the trade price. */
        [[nodiscard]] Price getPrice() const { return mPrice; }
        /** @brief Gets the trade timestamp. */
        [[nodiscard]] Timestamp getTimestamp() const { return mTimestamp; }

    private:
        TradeID mTradeId;           ///< Unique identifier for the trade.
        Instrument mOrderSymbol;    ///< Financial instrument symbol.
        OrderID mBuyOrderId;        ///< The ID of the buy order involved in the trade.
        OrderID mSellOrderId;       ///< The ID of the sell order involved in the trade.
        Quantity mQuantity;         ///< The quantity of the trade.
        Price mPrice;               ///< The price at which the trade was executed.
        Timestamp mTimestamp;       ///< The timestamp of when the trade occurred.
    };
}
