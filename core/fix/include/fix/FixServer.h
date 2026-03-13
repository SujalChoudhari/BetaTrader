#pragma once

#include "common_fix/ExecutionReport.h"
#include "common_fix/MarketDataIncrementalRefresh.h"
#include "common_fix/MarketDataSnapshotFullRefresh.h"
#include "data/SequenceRepository.h"
#include "fix/FixSession.h"
#include "fix/FixSessionManager.h"
#include "trading_core/TradingCore.h"
#include <asio.hpp>
#include <map>
#include <memory>
#include <string>

namespace fix {

    class FixServer {
    public:
        FixServer(asio::io_context& ioContext, short port,
                  trading_core::TradingCore& tradingCore,
                  ::data::SequenceRepository* seqRepo = nullptr);

        void run();
        void stop();

        void onExecutionReport(const ExecutionReport& report);

        void onMarketDataSnapshotFullRefresh(
                const MarketDataSnapshotFullRefresh& snapshot);

        void onMarketDataIncrementalRefresh(
                const MarketDataIncrementalRefresh& refresh);

        void registerSession(const std::shared_ptr<FixSession>& session);

        void unregisterSession(uint32_t sessionId);

        FixSessionManager& getManager() { return mSessionManager; }

    private:
        void doAccept();

        asio::io_context& mIoContext;
        asio::ip::tcp::acceptor mAcceptor;
        asio::ip::tcp::socket mSocket;
        trading_core::TradingCore& mTradingCore;
        FixSessionManager mSessionManager;
        std::map<uint32_t, std::shared_ptr<FixSession>> mSessions;
        uint32_t mNextSessionId = 1;
        std::map<std::string, std::vector<std::weak_ptr<FixSession>>>
                mMarketDataSubscriptions;
    };

} // namespace fix
