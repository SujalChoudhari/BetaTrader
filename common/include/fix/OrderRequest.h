#pragma once
#include "Types.h"
#include "common/Types.h"

namespace fix {
    /**
     * @class OrderRequest
     * @brief Represents a FIX New Order - Single (35=D) message.
     *
     * This class encapsulates the fields required to request a new order
     * in the FIX protocol.
     */
    class OrderRequest {
    public:
        /**
         * @brief Constructs a new OrderRequest object.
         * @param senderCompId The sender's component ID (Tag 49).
         * @param targetCompId The target's component ID (Tag 56).
         * @param msgSeqNum The message sequence number (Tag 34).
         * @param clientOrderId The client-assigned order ID (Tag 11).
         * @param symbol The financial instrument symbol (Tag 55).
         * @param side The side of the order (Buy/Sell) (Tag 54).
         * @param type The type of the order (Limit/Market) (Tag 40).
         * @param timeInForce The time in force for the order (Tag 59).
         * @param quantity The quantity of the order (Tag 38).
         * @param price The price of the order (Tag 44).
         * @param stopPrice The stop price for stop or stop-limit orders (Tag 99).
         * @param settlementType The settlement type (Tag 63).
         * @param settlementDate The settlement date (Tag 64).
         */
        OrderRequest(const CompID &senderCompId,
                     const CompID &targetCompId,
                     const SequenceNumber msgSeqNum,
                     const ClientOrderID &clientOrderId,
                     const Symbol &symbol,
                     const common::OrderSide side,
                     const common::OrderType type,
                     const common::TimeInForce timeInForce,
                     const Quantity quantity,
                     const Price price,
                     const Price stopPrice,
                     const uint8_t settlementType,
                     const uint32_t settlementDate)
            : mSenderCompId(senderCompId),
              mTargetCompId(targetCompId),
              mMessageSequenceNumber(msgSeqNum),
              mClientOrderId(clientOrderId),
              mSymbol(symbol),
              mSide(side),
              mType(type),
              mTimeInForce(timeInForce),
              mQuantity(quantity),
              mPrice(price),
              mStopPrice(stopPrice),
              mSettlementType(settlementType),
              mSettlementDate(settlementDate) {
        }

        /** @brief Gets the sender's component ID (Tag 49). */
        [[nodiscard]] const CompID &getSenderCompId() const { return mSenderCompId; }
        /** @brief Gets the target's component ID (Tag 56). */
        [[nodiscard]] const CompID &getTargetCompId() const { return mTargetCompId; }
        /** @brief Gets the message sequence number (Tag 34). */
        [[nodiscard]] SequenceNumber getMsgSeqNum() const { return mMessageSequenceNumber; }
        /** @brief Gets the client-assigned order ID (Tag 11). */
        [[nodiscard]] const ClientOrderID &getClientOrderId() const { return mClientOrderId; }
        /** @brief Gets the financial instrument symbol (Tag 55). */
        [[nodiscard]] const Symbol &getSymbol() const { return mSymbol; }
        /** @brief Gets the side of the order (Buy/Sell) (Tag 54). */
        [[nodiscard]] common::OrderSide getSide() const { return mSide; }
        /** @brief Gets the type of the order (Limit/Market) (Tag 40). */
        [[nodiscard]] common::OrderType getType() const { return mType; }
        /** @brief Gets the time in force for the order (Tag 59). */
        [[nodiscard]] common::TimeInForce getTimeInForce() const { return mTimeInForce; }
        /** @brief Gets the quantity of the order (Tag 38). */
        [[nodiscard]] Quantity getQuantity() const { return mQuantity; }
        /** @brief Gets the price of the order (Tag 44). */
        [[nodiscard]] Price getPrice() const { return mPrice; }
        /** @brief Gets the stop price for stop or stop-limit orders (Tag 99). */
        [[nodiscard]] Price getStopPrice() const { return mStopPrice; }
        /** @brief Gets the settlement type (Tag 63). */
        [[nodiscard]] uint8_t getSettlementType() const { return mSettlementType; }
        /** @brief Gets the settlement date (Tag 64). */
        [[nodiscard]] uint32_t getSettlementDate() const { return mSettlementDate; }

    private:
        // Session
        CompID mSenderCompId; ///< Tag 49: Sender's component ID.
        CompID mTargetCompId; ///< Tag 56: Target's component ID.
        SequenceNumber mMessageSequenceNumber; ///< Tag 34: Message sequence number.

        // Business
        ClientOrderID mClientOrderId; ///< Tag 11: Client-assigned order ID.
        Symbol mSymbol; ///< Tag 55: Financial instrument symbol.
        common::OrderSide mSide; ///< Tag 54: Side of the order (Buy/Sell).
        common::OrderType mType; ///< Tag 40: Type of the order (Limit/Market).
        common::TimeInForce mTimeInForce; ///< Tag 59: Time in force for the order.
        Quantity mQuantity; ///< Tag 38: Quantity of the order.
        Price mPrice; ///< Tag 44: Price of the order.
        Price mStopPrice; ///< Tag 99: Stop price (optional).

        // Settlement
        uint8_t mSettlementType; ///< Tag 63: Settlement type.
        uint32_t mSettlementDate; ///< Tag 64: Settlement date (YYYYMMDD).
    };
}
