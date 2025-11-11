/**
 * @file Types.h
 * @brief Defines type aliases for FIX protocol fields.
 *
 * This file provides type aliases for various data types used in FIX messages,
 * ensuring consistency and clarity throughout the application. These types
 * correspond to specific FIX tag data types.
 */

#pragma once
#include "common/Types.h"

namespace fix {
    /** @brief Type alias for a component ID (e.g., SenderCompID, TargetCompID).
     */
    using CompID = uint32_t;
    /** @brief Type alias for a message sequence number. */
    using SequenceNumber = uint64_t;
    /** @brief Type alias for a client-assigned order ID. */
    using ClientOrderID = uint64_t;
    /** @brief Type alias for an exchange-assigned order ID. */
    using ExchangeOrderID = uint64_t;
    /** @brief Type alias for a price field, inheriting from the common Price
     * type. */
    using Price = common::Price;
    /** @brief Type alias for a quantity field, inheriting from the common
     * Quantity type. */
    using Quantity = common::Quantity;
    /** @brief Type alias for a symbol field, inheriting from the common Symbol
     * type. */
    using Symbol = common::Symbol;
} // namespace fix
