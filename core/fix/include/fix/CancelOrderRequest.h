/**
 * @file CancelOrderRequest.h
 * @brief Defines the structure for a FIX Order Cancel Request.
 */

#pragma once

#include "common/Types.h"
#include "common_fix/Types.h"
#include <chrono>
#include <string>

namespace fix {

    /**
     * @brief Represents a parsed FIX Order Cancel Request message.
     *
     * This struct encapsulates the necessary fields from a raw FIX message
     * for canceling an existing order. It serves as an intermediate
     * representation before being converted into a trading core command.
     */
    struct CancelOrderRequest {
        /** @brief Client-assigned Order ID (ClOrdID, FIX Tag 11).
         *         Unique identifier for the cancel request. */
        fix::ClientOrderID clOrdID;
        /** @brief Original Order ID (OrderID, FIX Tag 37).
         *         The ID of the order to be canceled. */
        fix::ExchangeOrderID orderID;
        /** @brief Trading symbol of the order (Symbol, FIX Tag 55). */
        fix::Symbol symbol;
        /** @brief Side of the order (Side, FIX Tag 54).
         *         '1' = Buy, '2' = Sell. */
        common::OrderSide side;
        /** @brief Time of the transaction (TransactTime, FIX Tag 60).
         *         Timestamp when the cancel request was initiated. */
        std::chrono::time_point<std::chrono::system_clock> transactTime;
    };

} // namespace fix
