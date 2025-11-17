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
        // Simplified constructor
        OrderRequest(
            const common::ClientID& clientId,
            const ClientOrderID& clientOrderId,
            const Symbol& symbol,
            common::OrderSide side,
            double quantity,
            double price
        ) : clientId(clientId), clientOrderId(clientOrderId), symbol(symbol), side(side), quantity(quantity), price(price) {}

        common::ClientID clientId;
        ClientOrderID clientOrderId;
        Symbol symbol;
        common::OrderSide side;
        double quantity;
        double price;
    };
} // namespace fix
