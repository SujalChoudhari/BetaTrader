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
         * @param clientId The client identifier.
         * @param clientOrderId The client-assigned order ID (ClOrdID, FIX Tag 11).
         * @param symbol The trading symbol (Symbol, FIX Tag 55).
         * @param side The side of the order (Side, FIX Tag 54).
         * @param quantity The quantity of the order (OrderQty, FIX Tag 38).
         * @param price The price of the order (Price, FIX Tag 44).
         */
        OrderRequest(
            const common::ClientID& clientId,
            const ClientOrderID& clientOrderId,
            const Symbol& symbol,
            common::OrderSide side,
            fix::Quantity quantity,
            fix::Price price
        ) : clientId(clientId), clientOrderId(clientOrderId), symbol(symbol), side(side), quantity(quantity), price(price) {}

        /** @brief The client identifier. */
        common::ClientID clientId;
        /** @brief Client-assigned Order ID (ClOrdID, FIX Tag 11). */
        ClientOrderID clientOrderId;
        /** @brief Trading symbol (Symbol, FIX Tag 55). */
        Symbol symbol;
        /** @brief Side of the order (Side, FIX Tag 54). */
        common::OrderSide side;
        /** @brief Quantity of the order (OrderQty, FIX Tag 38). */
        fix::Quantity quantity;
        /** @brief Price of the order (Price, FIX Tag 44). */
        fix::Price price;
    };
} // namespace fix
