//
// Created by sujal on 21-10-2025.
//

#pragma once
#include "Time.h"
#include "Types.h"

namespace common {
    /**
     * Order class
     * Data model to store and use the Orders
     */
    class Order {
    public:
        Order(const OrderID id,
              const Symbol symbol,
              ClientID client,
              const OrderSide side,
              const OrderType type,
              const Quantity quantity,
              const Price price,
              const Timestamp ts)
            : mId(id),
              mSymbol(symbol),
              mClientId(std::move(client)),
              mOrderSide(side),
              mOrderType(type),
              mOriginalQuantity(quantity),
              mRemainingQuantity(quantity),
              mPrice(price),
              mTimestamp(ts),
              mOrderStatus(common::OrderStatus::New) {
        }

    public:
        [[nodiscard]] const OrderID &getId() const { return mId; }
        [[nodiscard]] const Symbol &getSymbol() const { return mSymbol; }
        [[nodiscard]] const ClientID &getClientId() const { return mClientId; }
        [[nodiscard]] OrderSide getSide() const { return mOrderSide; }
        [[nodiscard]] Price getPrice() const { return mPrice; }
        [[nodiscard]] Timestamp getTimestamp() const { return mTimestamp; }
        [[nodiscard]] OrderStatus getStatus() const { return mOrderStatus; }
        [[nodiscard]] OrderType getOrderType() const { return mOrderType; }
        [[nodiscard]] Quantity getOriginalQuantity() const { return mOriginalQuantity; }
        [[nodiscard]] Quantity getRemainingQuantity() const { return mRemainingQuantity; }

        void setRemainingQuantity(const Quantity qty) { mRemainingQuantity = qty; }
        void setStatus(const OrderStatus status) { mOrderStatus = status; }

    private:
        OrderID mId;
        Symbol mSymbol;
        ClientID mClientId;
        OrderSide mOrderSide;
        OrderType mOrderType;

        Quantity mOriginalQuantity;
        Quantity mRemainingQuantity;

        Price mPrice;
        Timestamp mTimestamp;

        OrderStatus mOrderStatus;
    };


    using OrderPtr = std::shared_ptr<Order>;
}
