//
// Created by sujal on 21-10-2025.
//

#pragma once
#include "Time.h"
#include "Types.h"
#include <string> // Include for std::string

namespace common {
    /**
     * @class Order
     * @brief Represents a single trading order in the system.
     *
     * This class is a data model that encapsulates all the properties of an
     * order, such as its ID, symbol, client, side, type, quantity, price, and
     * status.
     */
    class Order {
    public:
        /**
         * @brief Constructs a new Order object.
         * @param clientOrderId The unique identifier for the order given by the client.
         * @param coreOrderId The unique identifier for the order assigned by the core.
         * @param symbol The financial instrument symbol.
         * @param client The identifier of the client placing the order.
         * @param senderCompID The SenderCompID from the FIX message, identifying the actual trader.
         * @param side The side of the order (Buy or Sell).
         * @param type The type of the order (Limit or Market).
         * @param timeInForce The time in force of the order.
         * @param quantity The original quantity of the order.
         * @param price The price of the order.
         * @param ts The timestamp of when the order was created.
         */
        Order(const OrderID clientOrderId, const OrderID coreOrderId, const Symbol symbol, ClientID client,
              std::string senderCompID, // New parameter
              const OrderSide side, const OrderType type,
              const TimeInForce timeInForce, const Quantity quantity,
              const Price price, const Timestamp ts)
            : mClientOrderId(clientOrderId), mCoreOrderId(coreOrderId), mSymbol(symbol), mClientId(std::move(client)),
              mSenderCompID(std::move(senderCompID)), // Initialize new member
              mOrderSide(side), mOrderType(type), mTimeInForce(timeInForce),
              mOriginalQuantity(quantity), mRemainingQuantity(quantity),
              mPrice(price), mTimestamp(ts),
              mOrderStatus(common::OrderStatus::New)
        {}

    public:
        /** @brief Gets the order's core-provided identifier. This is the primary ID for internal tracking. */
        [[nodiscard]] const OrderID& getId() const
        {
            return mCoreOrderId;
        }
        /** @brief Gets the order's client-provided identifier. */
        [[nodiscard]] const OrderID& getClientOrderId() const
        {
            return mClientOrderId;
        }
        /** @brief Gets the financial instrument symbol. */
        [[nodiscard]] const Symbol& getSymbol() const { return mSymbol; }
        /** @brief Gets the client's identifier (session ID). */
        [[nodiscard]] const ClientID& getClientId() const { return mClientId; }
        /** @brief Gets the SenderCompID, identifying the actual trader. */
        [[nodiscard]] const std::string& getSenderCompID() const { return mSenderCompID; } // New getter
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
        /** @brief Gets the time in force of the order. */
        [[nodiscard]] TimeInForce getTimeInForce() const
        {
            return mTimeInForce;
        }
        /** @brief Gets the original quantity of the order. */
        [[nodiscard]] Quantity getOriginalQuantity() const
        {
            return mOriginalQuantity;
        }
        /** @brief Gets the remaining quantity of the order. */
        [[nodiscard]] Quantity getRemainingQuantity() const
        {
            return mRemainingQuantity;
        }

        /** @brief Sets the order's client-provided identifier. */
        void setClientOrderId(const OrderID id) { mClientOrderId = id; }
        /** @brief Sets the order's core-provided identifier. */
        void setCoreOrderId(const OrderID id) { mCoreOrderId = id; }
        /** @brief Sets the remaining quantity of the order. */
        void setRemainingQuantity(const Quantity qty)
        {
            mRemainingQuantity = qty;
        }
        /** @brief Sets the status of the order. */
        void setStatus(const OrderStatus status) { mOrderStatus = status; }
        /** @brief Sets the price of the order. */
        void setPrice(const Price price) { mPrice = price; }
        /** @brief Sets the original and remaining quantity of the order. */
        void setOriginalQuantity(const Quantity qty)
        {
            mOriginalQuantity = qty;
            mRemainingQuantity = qty;
        }

        /** @brief Sets the timestamp of the order. */
        void setTimestamp(const Timestamp ts) { mTimestamp = ts; }

    private:
        OrderID mClientOrderId; ///< Unique identifier for the order given by client
        OrderID mCoreOrderId;   ///< Unique identifier for the order given by core
        Symbol mSymbol;         ///< Financial instrument symbol.
        ClientID mClientId;     ///< Identifier of the client placing the order (session ID).
        std::string mSenderCompID; ///< SenderCompID from the FIX message, identifying the actual trader.
        OrderSide mOrderSide;   ///< The side of the order (Buy or Sell).
        OrderType mOrderType;   ///< The type of the order (Limit or Market).
        TimeInForce mTimeInForce; ///< The time in force of the order.
        Quantity mOriginalQuantity; ///< The original quantity of the order.
        Quantity mRemainingQuantity; ///< The quantity of the order that has not yet been filled.

        Price mPrice;           ///< The price of the order.
        Timestamp mTimestamp;   ///< The timestamp of when the order was created.

        OrderStatus mOrderStatus; ///< The current status of the order.
    };
} // namespace common
