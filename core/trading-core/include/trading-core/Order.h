//
// Created by sujal on 21-10-2025.
//

#pragma once
#include "common/Time.h"
#include "common/Types.h"

namespace trading_core {
    /**
     * Order class
     * Data model to store and use the Orders
     */
    class Order {
    public:
        Order(common::OrderID id,
              common::Symbol symbol,
              common::ClientID client,
              const common::OrderSide side,
              const common::OrderType type,
              const common::Quantity qty,
              const common::Price price,
              const common::Timestamp ts)
            : mId(id),
              mSymbol(std::move(symbol)),
              mClientId(std::move(client)),
              mOrderSide(side),
              mOrderType(type),
              mOriginalQuantity(qty),
              mPendingQuantity(qty),
              mPrice(price),
              mTimestamp(ts),
              mOrderStatus(common::OrderStatus::New) {
        }

    public:
        [[nodiscard]] const common::OrderID &getId() const { return mId; }
        [[nodiscard]] const common::Symbol &getSymbol() const { return mSymbol; }
        [[nodiscard]] common::OrderSide getSide() const { return mOrderSide; }
        [[nodiscard]] common::Price getPrice() const { return mPrice; }
        [[nodiscard]] common::Quantity getRemainingQty() const { return mPendingQuantity; }
        [[nodiscard]] common::Timestamp getTimestamp() const { return mTimestamp; }
        [[nodiscard]] common::OrderStatus getStatus() const { return mOrderStatus; }
        [[nodiscard]] common::OrderType getOrderType() const { return mOrderType; }
        [[nodiscard]] common::Quantity getOriginalQuantity() const { return mOriginalQuantity; }
        [[nodiscard]] common::Quantity getPendingQuantity() const { return mPendingQuantity; }

        void setRemainingQty(const common::Quantity qty) { mPendingQuantity = qty; }
        void setStatus(const common::OrderStatus status) { mOrderStatus = status; }

    private:
        common::OrderID mId;
        common::Symbol mSymbol;
        common::ClientID mClientId;
        common::OrderSide mOrderSide;
        common::OrderType mOrderType;

        common::Quantity mOriginalQuantity;
        common::Quantity mPendingQuantity;

        common::Price mPrice;
        common::Timestamp mTimestamp;

        common::OrderStatus mOrderStatus;
    };


    using OrderPtr = std::shared_ptr<Order>;
}
