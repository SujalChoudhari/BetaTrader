/**
 * @file FixMessageParser.h
 * @brief Declares the FixMessageParser class for parsing incoming FIX messages for the client.
 */

#pragma once

#include "common_fix/ExecutionReport.h"
#include "common_fix/MarketDataSnapshotFullRefresh.h"
#include "common_fix/MarketDataIncrementalRefresh.h"
#include <optional>
#include <string>
#include <variant>

namespace fix_client {

    /**
     * @brief A variant representing the possible parsed messages received by the client.
     */
    using ParsedFixMessage = std::variant<
        std::monostate, // Represents a message we don't handle deeply (e.g. Heartbeat, Logon Ack)
        fix::ExecutionReport,
        fix::MarketDataSnapshotFullRefresh,
        fix::MarketDataIncrementalRefresh
    >;

    /**
     * @class FixMessageParser
     * @brief Parses raw FIX messages received from the server into strongly-typed structures.
     * 
     * Handles '8' (ExecutionReport), 'W' (MarketDataSnapshotFullRefresh), and 'X' (MarketDataIncrementalRefresh).
     */
    class FixMessageParser {
    public:
        /**
         * @brief Parses a raw FIX message string.
         * 
         * @param fixMessage The raw FIX message including SOH delimiters.
         * @return A parsed variant representing the message. Returns monostate if unparseable or irrelevant.
         */
        static ParsedFixMessage parse(const std::string& fixMessage);

    private:
        static std::optional<fix::ExecutionReport> parseExecutionReport(const std::string& fixMessage);
        static std::optional<fix::MarketDataSnapshotFullRefresh> parseMarketDataSnapshot(const std::string& fixMessage);
        static std::optional<fix::MarketDataIncrementalRefresh> parseMarketDataIncremental(const std::string& fixMessage);
    };

} // namespace fix_client
