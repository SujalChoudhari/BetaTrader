/**
 * @file ModifyOrder.h
 * @brief Defines the structure for a FIX Order Cancel Replace Request.
 */

#pragma once

#include <string>
#include <chrono>
#include "fix/Types.h"
#include "common/Types.h"

namespace fix
{

/**
 * @brief Represents a FIX Order Cancel Replace Request message.
 *
 * This struct encapsulates the necessary fields for modifying an existing order
 * in the FIX protocol.
 */
struct ModifyOrder
{
    /** @brief Client-assigned Order ID (ClOrdID, FIX Tag 11).
     *         Unique identifier for the modification request. */
    std::string clOrdID;
    /** @brief Original Client Order ID (OrigClOrdID, FIX Tag 41).
     *         The Client Order ID of the order being replaced. */
    std::string origClOrdID;
    /** @brief Original Order ID (OrderID, FIX Tag 37).
     *         The ID of the order to be modified. */
    std::string orderID;
    /** @brief Trading symbol of the order (Symbol, FIX Tag 55). */
    fix::Symbol symbol;
    /** @brief Side of the order (Side, FIX Tag 54).
     *         '1' = Buy, '2' = Sell. */
    common::OrderSide side;
    /** @brief Quantity of the order (OrderQty, FIX Tag 38). */
    fix::Quantity orderQty;
    /** @brief Type of the order (OrdType, FIX Tag 40).
     *         '1' = Market, '2' = Limit. */
    common::OrderType ordType;
    /** @brief Price of the order (Price, FIX Tag 44).
     *         Required for Limit orders. */
    fix::Price price;
    /** @brief Time of the transaction (TransactTime, FIX Tag 60).
     *         Timestamp when the modification request was initiated. */
    std::chrono::time_point<std::chrono::system_clock> transactTime;
};

} // namespace fix
