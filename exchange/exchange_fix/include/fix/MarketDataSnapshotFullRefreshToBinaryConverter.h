/**
 * @file MarketDataSnapshotFullRefreshToBinaryConverter.h
 * @brief Declares the MarketDataSnapshotFullRefreshToBinaryConverter class for converting MarketDataSnapshotFullRefresh objects to FIX messages.
 */

#pragma once

#include "common_fix/MarketDataSnapshotFullRefresh.h"
#include <string>

namespace fix
{

/**
 * @brief Converts a `MarketDataSnapshotFullRefresh` object into a raw FIX message string.
 *
 * This class provides a static method to serialize a structured `MarketDataSnapshotFullRefresh`
 * object into a FIX "Market Data Snapshot Full Refresh" (MsgType=W) message string.
 */
class MarketDataSnapshotFullRefreshToBinaryConverter
{
public:
    /**
     * @brief Converts a `MarketDataSnapshotFullRefresh` object into a raw FIX message string.
     *
     * Serializes the given `MarketDataSnapshotFullRefresh` object into a complete,
     * valid FIX message string, including header, body, and trailer.
     *
     * @param snapshot The `MarketDataSnapshotFullRefresh` object to serialize.
     * @return A string containing the complete, valid FIX message.
     */
    static std::string convert(const MarketDataSnapshotFullRefresh& snapshot);
};

} // namespace fix
