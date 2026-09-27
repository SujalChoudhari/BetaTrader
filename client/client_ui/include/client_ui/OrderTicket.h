#pragma once

#include "common/Instrument.h"
#include <cmath>
#include <optional>
#include <string>

namespace client_ui {

    struct OrderTicket {
        std::string symbol;
        char side;
        double price;
        int quantity;
        char orderType;
        char timeInForce;
    };

    /**
     * Returns a user-facing validation error, or std::nullopt for a valid
     * ticket. The checks mirror the fields the FIX order-entry panel can
     * submit.
     */
    inline std::optional<std::string>
    validateOrderTicket(const OrderTicket& ticket)
    {
        if (ticket.symbol.empty()) { return "Symbol is required."; }

        try {
            common::from_string(ticket.symbol);
        }
        catch (const std::invalid_argument&) {
            return "Symbol is not supported.";
        }

        if (ticket.side != '1' && ticket.side != '2') {
            return "Side must be BUY or SELL.";
        }

        if (ticket.quantity <= 0) { return "Quantity must be positive."; }

        if (ticket.orderType != '1' && ticket.orderType != '2') {
            return "Order type is unsupported.";
        }

        if (ticket.orderType == '2'
            && (!std::isfinite(ticket.price) || ticket.price <= 0.0)) {
            return "Limit orders require a positive price.";
        }

        if (ticket.timeInForce != '0' && ticket.timeInForce != '1'
            && ticket.timeInForce != '3' && ticket.timeInForce != '4') {
            return "Time-in-force is unsupported.";
        }

        return std::nullopt;
    }

} // namespace client_ui
