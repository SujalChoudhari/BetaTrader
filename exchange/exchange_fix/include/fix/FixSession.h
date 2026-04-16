#pragma once

#include "common/Instrument.h"
#include "common_fix/BusinessMessageReject.h"
#include "common_fix/ExecutionReport.h"
#include "common_fix/MarketDataIncrementalRefresh.h"
#include "common_fix/MarketDataSnapshotFullRefresh.h"
#include "common_fix/Reject.h"
#include <exchange_app/TradingCore.h>
#include <asio.hpp>
#include <cstdint>
#include <deque>
#include <memory>
#include <string>
#include <unordered_map>

namespace fix {
    class FixServer; // Forward declaration

    class FixSession : public std::enable_shared_from_this<FixSession> {
    public:
        FixSession(asio::ip::tcp::socket socket, FixServer& server, // Add server reference
                   trading_core::TradingCore& tradingCore, uint32_t sessionId);

        void start();
        void stop();

        void sendExecutionReport(const ExecutionReport& report);
        void sendMarketDataSnapshotFullRefresh(
                const MarketDataSnapshotFullRefresh& snapshot);
        void sendMarketDataIncrementalRefresh(
                const MarketDataIncrementalRefresh& refresh);
        void sendReject(const Reject& reject);
        void sendBusinessMessageReject(const BusinessMessageReject& bizReject);

        [[nodiscard]] uint32_t getSessionID() const;

    private:
        void doRead();
        void doWrite(std::shared_ptr<std::string> message);
        void handleFixMessage(const std::string& fixMessage, char msgType);
        void handleCancelOrderRequest(const std::string& fixMessage);
        void handleModifyOrderRequest(const std::string& fixMessage);
        void handleMarketDataRequest(const std::string& fixMessage);

        static constexpr size_t MaxLength = 8192;
        asio::ip::tcp::socket mSocket;
        FixServer& mServer; // Store server reference
        trading_core::TradingCore& mTradingCore;
        std::vector<char> mData;
        std::string mReadBuffer;
        uint32_t mSessionId;
        std::unordered_map<common::Symbol, std::string> mMarketDataReqIdMap;
    };

} // namespace fix
