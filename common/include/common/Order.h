//
// Created by sujal on 21-10-2025.
//

#pragma once
#include "Time.h"
#include "Types.h"

namespace common {
    /**
     * @class Order
     * @brief Represents a single trading order in the system.
     *
     * This class is a data model that encapsulates all the properties of an order,
     * such as its ID, symbol, client, side, type, quantity, price, and status.
     */
    class Order {
    public:
        /**
         * @brief Constructs a new Order object.
         * @param id The unique identifier for the order.
         * @param symbol The financial instrument symbol.
         * @param client The identifier of the client placing the order.
         * @param side The side of the order (Buy or Sell).
         * @param type The type of the order (Limit or Market).
         * @param quantity The original quantity of the order.
         * @param price The price of the order.
         * @param ts The timestamp of when the order was created.
         */
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
        /** @brief Gets the order's unique identifier. */
        [[nodiscard]] const OrderID &getId() const { return mId; }
        /** @brief Gets the financial instrument symbol. */
        [[nodiscard]] const Symbol &getSymbol() const { return mSymbol; }
        /** @brief Gets the client's identifier. */
        [[nodiscard]] const ClientID &getClientId() const { return mClientId; }
        /** @brief Gets the order side (Buy/Sell). */
        [[nodiscard]] OrderSide getSide() const { return mOrderSide; }
        /** @brief Gets the order price. */
        [[nodiscard]] Price getPrice() const { return mPrice; }
        /** @brief Gets the order creation timestamp. */
        [[nodiscard]] Timestamp getTimestamp() const { return mTimestamp; }
        /** @brief Gets the current status of the order. */
        [[nodiscard]] OrderStatus getStatus() const { return mOrderStatus; }
        /** @brief Gets the order type (Limit/Market). */
        [[nodiscard]] OrderType getOrderType() const { return mOrderType; }
        /** @brief Gets the original quantity of the order. */
        [[nodiscard]] Quantity getOriginalQuantity() const { return mOriginalQuantity; }
        /** @brief Gets the remaining quantity of the order. */
        [[nodiscard]] Quantity getRemainingQuantity() const { return mRemainingQuantity; }

        /** @brief Sets the order's unique identifier. */
        void setId(const OrderID id) { mId = id; }
        /** @brief Sets the remaining quantity of the order. */
        void setRemainingQuantity(const Quantity qty) { mRemainingQuantity = qty; }
        /** @brief Sets the status of the order. */
        void setStatus(const OrderStatus status) { mOrderStatus = status; }
        /** @brief Sets the price of the order. */
        void setPrice(const Price price) { mPrice = price; }
        /** @brief Sets the original and remaining quantity of the order. */
        void setOriginalQuantity(const Quantity qty) {
            mOriginalQuantity = qty;
            mRemainingQuantity = qty;
        }
        /** @brief Sets the timestamp of the order. */
        void setTimestamp(const Timestamp ts) { mTimestamp = ts; }

    private:
        OrderID mId;                ///< Unique identifier for the order.
        Symbol mSymbol;             ///< Financial instrument symbol.
        ClientID mClientId;         ///< Identifier of the client placing the order.
        OrderSide mOrderSide;       ///< The side of the order (Buy or Sell).
        OrderType mOrderType;       ///< The type of the order (Limit or Market).

        Quantity mOriginalQuantity; ///< The original quantity of the order.
        Quantity mRemainingQuantity;///< The quantity of the order that has not yet been filled.

        Price mPrice;               ///< The price of the order.
        Timestamp mTimestamp;       ///< The timestamp of when the order was created.

        OrderStatus mOrderStatus;   ///< The current status of the order.
    };


}
