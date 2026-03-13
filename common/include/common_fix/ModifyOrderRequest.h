/**
 * @file ModifyOrderRequest.h
 * @brief Defines the structure for a FIX Order Cancel Replace Request.
 */

#pragma once

#include "common/Types.h"
#include "common_fix/Types.h"
#include <chrono>
#include <string>

namespace fix
{

/**
 * @brief Represents a parsed FIX Order Cancel Replace Request (Modify Order) message.
 *
 * This struct encapsulates the necessary fields from a raw FIX message
 * for modifying an existing order. It serves as an intermediate representation
 * before being converted into a trading core command.
 */
struct ModifyOrderRequest
{
    /** @brief Client-assigned Order ID (ClOrdID, FIX Tag 11).
     *         Unique identifier for the modification request. */
    fix::ClientOrderID clOrdID;
    /** @brief Original Client Order ID (OrigClOrdID, FIX Tag 41).
     *         The Client Order ID of the order being replaced. */
    fix::ClientOrderID origClOrdID;
    /** @brief Original Order ID (OrderID, FIX Tag 37).
     *         The ID of the order to be modified. */
    fix::ExchangeOrderID orderID;
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
