#pragma once

#include "common_fix/Types.h" // For fix::Symbol
#include <string>
#include <vector>

namespace fix {

    /**
     * @brief Represents a parsed FIX Market Data Request message.
     *
     * This struct encapsulates the necessary fields from a raw FIX message
     * for requesting market data. It serves as an intermediate representation
     * before being converted into a trading core command or used for market
     * data subscription logic.
     */
    struct MarketDataRequest {
        std::string mdReqID;
        char subscriptionRequestType; // '0' = Snapshot, '1' = Snapshot +
                                      // Updates, '2' = Unsubscribe
        int marketDepth; // '0' = Full Book, '1' = Top of Book / Level 1
        std::vector<fix::Symbol> symbols; // Using fix::Symbol
    };

} // namespace fix
