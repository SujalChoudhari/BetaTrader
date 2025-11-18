/**
 * @file MarketDataIncrementalRefreshToBinaryConverter.h
 * @brief Declares the MarketDataIncrementalRefreshToBinaryConverter class for converting MarketDataIncrementalRefresh objects to FIX messages.
 */

#pragma once

#include "common_fix/MarketDataIncrementalRefresh.h"
#include <string>

namespace fix
{

/**
 * @brief Converts a `MarketDataIncrementalRefresh` object into a raw FIX message string.
 *
 * This class provides a static method to serialize a structured `MarketDataIncrementalRefresh`
 * object into a FIX "Market Data Incremental Refresh" (MsgType=X) message string.
 */
class MarketDataIncrementalRefreshToBinaryConverter
{
public:
    /**
     * @brief Converts a `MarketDataIncrementalRefresh` object into a raw FIX message string.
     *
     * Serializes the given `MarketDataIncrementalRefresh` object into a complete,
     * valid FIX message string, including header, body, and trailer.
     *
     * @param refresh The `MarketDataIncrementalRefresh` object to serialize.
     * @return A string containing the complete, valid FIX message.
     */
    static std::string convert(const MarketDataIncrementalRefresh& refresh);
};

} // namespace fix
