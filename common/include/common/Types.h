//
// Created by sujal on 21-10-2025.
//

#pragma once
#include <cstdint>
#include <string>

#include "Instrument.h"

namespace common {
    /**
     * Represents the very nature of the orders
     */
    enum class OrderSide: uint8_t {
        Buy,
        Sell,
    };

    /**
     * Represents the execution logic of the order
     * 1. Limit - Sets a limit, (this, or better price)
     * 2. Market - aggressive in nature, takes the best,
     *          guarantees the order but not the price.
     */
    enum class OrderType: uint8_t {
        Limit,
        Market
    };

    /**
     * Represents the lifecycle of the orders
     * 1. New - Not matched yet
     * 2. PartiallyFilled - Matched but was not able to complete the full requirement
     * 3. Filled - Matched and satisfied
     * 4. Cancelled - Aborted before Filled
     * 5. Rejected - Order was logged but never made to the engine, i.e. blocked
     */
    enum class OrderStatus: uint8_t {
        New,
        PartiallyFilled,
        Filled,
        Cancelled,
        Rejected
    };


    using Price = double;
    using Quantity = uint64_t;
    using OrderID = uint64_t;
    using TradeID = uint64_t;
    using Symbol = Instrument;
    using ClientID = std::string;
}
