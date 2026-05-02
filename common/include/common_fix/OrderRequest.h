#pragma once
#include "common/Types.h"
#include "common_fix/Types.h"
#include <string>

namespace fix {
    /**
     * @class OrderRequest
     * @brief Represents a parsed FIX New Order - Single (35=D) message.
     *
     * This class encapsulates the fields from a raw FIX message required to
     * request a new order. It serves as an intermediate representation before
     * being converted into a trading core command.
     */
    class OrderRequest {
    public:
        /**
         * @brief Constructs a new OrderRequest object.
         * @param senderCompID The sender's component ID (SenderCompID, FIX Tag 49).
         * @param clientOrderId The client-assigned order ID (ClOrdID, FIX Tag 11).
         * @param symbol The trading symbol (Symbol, FIX Tag 55).
         * @param side The side of the order (Side, FIX Tag 54).
         * @param quantity The quantity of the order (OrderQty, FIX Tag 38).
         * @param price The price of the order (Price, FIX Tag 44).
         */
        OrderRequest(
            const std::string& senderCompID,
            const ClientOrderID& clientOrderId,
            const Symbol& symbol,
            common::OrderSide side,
            common::OrderType orderType,
            fix::Quantity quantity,
            fix::Price price
        ) : senderCompID(senderCompID), clientOrderId(clientOrderId), symbol(symbol), side(side), orderType(orderType), quantity(quantity), price(price) {}

        /** @brief The sender's component ID (SenderCompID, FIX Tag 49). */
        std::string senderCompID;
        /** @brief Client-assigned Order ID (ClOrdID, FIX Tag 11). */
        ClientOrderID clientOrderId;
        /** @brief Trading symbol (Symbol, FIX Tag 55). */
        Symbol symbol;
        /** @brief Side of the order (Side, FIX Tag 54). */
        common::OrderSide side;
        /** @brief The type of the order (OrdType, FIX Tag 40). */
        common::OrderType orderType;
        /** @brief Quantity of the order (OrderQty, FIX Tag 38). */
        fix::Quantity quantity;
        /** @brief Price of the order (Price, FIX Tag 44). */
        fix::Price price;
    };
} // namespace fix
