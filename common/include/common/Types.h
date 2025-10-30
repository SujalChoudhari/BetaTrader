//
// Created by sujal on 21-10-2025.
//

#pragma once
#include <cstdint>
#include <string>

#include "Instrument.h"

namespace common {
    /**
     * @enum OrderSide
     * @brief Represents the side of an order (Buy or Sell).
     */
    enum class OrderSide: uint8_t {
        Buy,    ///< A buy order.
        Sell,   ///< a sell order.
    };

    /**
     * @enum OrderType
     * @brief Represents the execution logic of an order.
     */
    enum class OrderType: uint8_t {
        Limit,  ///< A limit order, which is executed at a specified price or better.
        Market  ///< A market order, which is executed immediately at the best available price.
    };

    /**
     * @enum OrderStatus
     * @brief Represents the lifecycle of an order.
     */
    enum class OrderStatus: uint8_t {
        New,             ///< The order has been created but not yet matched.
        PartiallyFilled, ///< The order has been partially matched.
        Filled,          ///< The order has been fully matched.
        Cancelled,       ///< The order has been cancelled before being fully matched.
        Rejected         ///< The order was rejected by the system and never entered the matching engine.
    };


    /** @brief A type alias for price values. */
    using Price = double;
    /** @brief A type alias for quantity values. */
    using Quantity = uint64_t;
    /** @brief A type alias for unique order identifiers. */
    using OrderID = uint64_t;
    /** @brief A type alias for unique trade identifiers. */
    using TradeID = uint64_t;
    /** @brief A type alias for financial instrument symbols. */
    using Symbol = Instrument;
    /** @brief A type alias for client identifiers. */
    using ClientID = std::string;
}
