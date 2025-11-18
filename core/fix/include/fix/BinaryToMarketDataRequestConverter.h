/**
 * @file BinaryToMarketDataRequestConverter.h
 * @brief Declares the BinaryToMarketDataRequestConverter class for converting FIX messages to MarketDataRequest objects.
 */

#pragma once

#include "fix/MarketDataRequest.h"
#include <optional>
#include <string>

namespace fix
{

/**
 * @brief Converts a raw FIX message string into a `MarketDataRequest` object.
 *
 * This class provides a static method to parse a FIX "Market Data Request" (MsgType=V)
 * message string and extract the relevant fields to construct a `MarketDataRequest`.
 */
class BinaryToMarketDataRequestConverter
{
public:
    /**
     * @brief Converts a raw FIX message string into a `MarketDataRequest` object.
     *
     * Parses the given FIX message string. If the message is a valid "Market Data Request"
     * and all required fields are present and correctly formatted, a `MarketDataRequest`
     * object is returned. Otherwise, `std::nullopt` is returned.
     *
     * @param fixMessage The raw FIX message string to convert.
     * @return An `std::optional<MarketDataRequest>` containing the parsed market data request
     *         if successful, or `std::nullopt` if parsing fails.
     */
    static std::optional<MarketDataRequest> convert(const std::string& fixMessage);
};

} // namespace fix
