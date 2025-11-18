#pragma once
#include "Types.h"
#include "common/Time.h"
#include "common/Types.h"

namespace fix {
    /**
     * @class ExecutionReport
     * @brief Represents a FIX Execution Report (35=8) message.
     *
     * This class encapsulates the fields used to report the status of an order
     * in the FIX protocol.
     */
    class ExecutionReport {
    public:
        /**
         * @brief Constructs a new ExecutionReport object.
         * @param senderCompId The sender's component ID (Tag 49).
         * @param targetCompId The target's component ID (Tag 56).
         * @param msgSeqNum The message sequence number (Tag 34).
         * @param exchangeOrderId The exchange-assigned order ID (Tag 37).
         * @param clientOrderId The client-assigned order ID (Tag 11).
         * @param executionId The execution ID (Tag 17).
         * @param status The order status (Tag 39).
         * @param text The text message (Tag 58).
         * @param symbol The financial instrument symbol (Tag 55).
         * @param side The side of the order (Buy/Sell) (Tag 54).
         * @param orderQty The total order quantity (Tag 38).
         * @param cumQty The cumulative filled quantity (Tag 14).
         * @param leavesQty The remaining quantity (Tag 151).
         * @param lastPrice The price of the last fill (Tag 31).
         * @param lastQty The quantity of the last fill (Tag 32).
         * @param transactTime The time of the transaction (Tag 60).
         */
        ExecutionReport(const CompID& senderCompId, const CompID& targetCompId,
                      const SequenceNumber msgSeqNum,
                      const ExchangeOrderID& exchangeOrderId,
                      const ClientOrderID& clientOrderId,
                      const std::string& executionId,
                      const common::OrderStatus status, const std::string& text,
                      const Symbol& symbol, const common::OrderSide side,
                      const Quantity orderQty, const Quantity cumQty,
                      const Quantity leavesQty, const Price lastPrice,
                      const Quantity lastQty,
                      const common::Timestamp& transactTime)
            : mSenderCompId(senderCompId), mTargetCompId(targetCompId),
              mMessageSequenceNumber(msgSeqNum),
              mExchangeOrderId(exchangeOrderId), mClientOrderId(clientOrderId),
              mExecutionId(executionId), mStatus(status), mText(text),
              mSymbol(symbol), mSide(side), mOrderQuantity(orderQty),
              mCumulativeQuantity(cumQty), mLeavesQuantity(leavesQty),
              mLastPrice(lastPrice), mLastQuantity(lastQty),
              mTransactionTime(transactTime)
        {}

        /** @brief Gets the sender's component ID (Tag 49). */
        [[nodiscard]] const CompID& getSenderCompId() const
        {
            return mSenderCompId;
        }
        /** @brief Gets the target's component ID (Tag 56). */
        [[nodiscard]] const CompID& getTargetCompId() const
        {
            return mTargetCompId;
        }
        /** @brief Gets the message sequence number (Tag 34). */
        [[nodiscard]] SequenceNumber getMessageSequenceNumber() const
        {
            return mMessageSequenceNumber;
        }
        /** @brief Gets the exchange-assigned order ID (Tag 37). */
        [[nodiscard]] const ExchangeOrderID& getExchangeOrderId() const
        {
            return mExchangeOrderId;
        }
        /** @brief Gets the client-assigned order ID (Tag 11). */
        [[nodiscard]] const ClientOrderID& getClientOrderId() const
        {
            return mClientOrderId;
        }
        /** @brief Gets the execution ID (Tag 17). */
        [[nodiscard]] const std::string& getExecutionId() const
        {
            return mExecutionId;
        }
        /** @brief Gets the order status (Tag 39). */
        [[nodiscard]] common::OrderStatus getStatus() const { return mStatus; }
        /** @brief Gets the text message (Tag 58). */
        [[nodiscard]] const std::string& getText() const { return mText; }
        /** @brief Gets the financial instrument symbol (Tag 55). */
        [[nodiscard]] const Symbol& getSymbol() const { return mSymbol; }
        /** @brief Gets the side of the order (Buy/Sell) (Tag 54). */
        [[nodiscard]] common::OrderSide getSide() const { return mSide; }
        /** @brief Gets the total order quantity (Tag 38). */
        [[nodiscard]] Quantity getOrderQuantity() const
        {
            return mOrderQuantity;
        }
        /** @brief Gets the cumulative filled quantity (Tag 14). */
        [[nodiscard]] Quantity getCumulativeQuantity() const
        {
            return mCumulativeQuantity;
        }
        /** @brief Gets the remaining quantity (Tag 151). */
        [[nodiscard]] Quantity getLeavesQuantity() const
        {
            return mLeavesQuantity;
        }
        /** @brief Gets the price of the last fill (Tag 31). */
        [[nodiscard]] Price getLastPrice() const { return mLastPrice; }
        /** @brief Gets the quantity of the last fill (Tag 32). */
        [[nodiscard]] Quantity getLastQuantity() const { return mLastQuantity; }
        /** @brief Gets the time of the transaction (Tag 60). */
        [[nodiscard]] const common::Timestamp& getTransactionTime() const
        {
            return mTransactionTime;
        }

    private:
        /** @brief Tag 49: Sender's component ID. */
        CompID mSenderCompId;
        /** @brief Tag 56: Target's component ID. */
        CompID mTargetCompId;
        /** @brief Tag 34: Message sequence number. */
        SequenceNumber mMessageSequenceNumber;

        /** @brief Tag 37: Exchange-assigned order ID. */
        ExchangeOrderID mExchangeOrderId;
        /** @brief Tag 11: Client-assigned order ID (echoed back). */
        ClientOrderID mClientOrderId;
        /** @brief Tag 17: Execution ID. */
        std::string mExecutionId;

        /** @brief Tag 39: Order status. */
        common::OrderStatus mStatus;
        /** @brief Tag 58: Text message (e.g., reject reason, notes). */
        std::string mText;

        /** @brief Tag 55: Financial instrument symbol. */
        Symbol mSymbol;
        /** @brief Tag 54: Side of the order (Buy/Sell). */
        common::OrderSide mSide;
        /** @brief Tag 38: Total order quantity. */
        Quantity mOrderQuantity;
        /** @brief Tag 14: Cumulative filled quantity. */
        Quantity mCumulativeQuantity;
        /** @brief Tag 151: Remaining quantity. */
        Quantity mLeavesQuantity;
        /** @brief Tag 31: Price of the last fill. */
        Price mLastPrice;
        /** @brief Tag 32: Quantity of the last fill. */
        Quantity mLastQuantity;
        /** @brief Tag 60: Time of the transaction. */
        common::Timestamp mTransactionTime;
    };
} // namespace fix
