/**
 * @file CancelOrder.h
 * @brief Defines the structure for a FIX Order Cancel Request.
 */

#pragma once

#include <string>
#include <chrono>
#include "fix/Types.h"
#include "common/Types.h"

namespace fix
{

/**
 * @brief Represents a FIX Order Cancel Request message.
 *
 * This struct encapsulates the necessary fields for canceling an existing order
 * in the FIX protocol.
 */
struct CancelOrder
{
    /** @brief Client-assigned Order ID (ClOrdID, FIX Tag 11).
     *         Unique identifier for the cancel request. */
    std::string clOrdID;
    /** @brief Original Order ID (OrderID, FIX Tag 37).
     *         The ID of the order to be canceled. */
    std::string orderID;
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
