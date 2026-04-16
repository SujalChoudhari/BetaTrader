#include "fix/FixServer.h"
#include "fix/FixSession.h"
#include "logging/Logger.h"

namespace fix {

    FixServer::FixServer(asio::io_context& ioContext, short port,
                         trading_core::TradingCore& tradingCore,
                         data::SequenceRepository* seqRepo)
        : mIoContext(ioContext),
          mAcceptor(ioContext,
                    asio::ip::tcp::endpoint(asio::ip::tcp::v4(), port)),
          mSocket(ioContext), mTradingCore(tradingCore), mSessionManager(seqRepo)
    {
        if (auto authRepo = mTradingCore.getAuthRepository()) {
            authRepo->loadValidClients([this](const std::vector<std::string>& clients) {
                mSessionManager.loadConfig(clients);
            });
        }

        mTradingCore.subscribeToExecutions(
            [this](const ExecutionReport& report) {
                onExecutionReport(report);
            }
        );

        mTradingCore.getMarketDataPublisher().subscribeToSnapshots(
            [this](const MarketDataSnapshotFullRefresh& snapshot) {
                onMarketDataSnapshotFullRefresh(snapshot);
            }
        );

        mTradingCore.getMarketDataPublisher().subscribeToIncrementals(
            [this](const MarketDataIncrementalRefresh& refresh) {
                onMarketDataIncrementalRefresh(refresh);
            }
        );
        
        doAccept();
    }

    void FixServer::run()
    {
        mIoContext.run();
    }

    void FixServer::stop()
    {
        if (mAcceptor.is_open()) {
            std::error_code ec;
            mAcceptor.close(ec);
        }
        for (auto& [id, session] : mSessions) {
            session->stop();
        }
        mSessions.clear();
        mSessionManager.getMutex().lock();
        // Clear runtime state
        mSessionManager.getMutex().unlock();
    }

    void FixServer::onExecutionReport(const ExecutionReport& report)
    {
        uint32_t sessionId = report.getTargetCompId();
        if (sessionId == 0) return; // Simulator or internal
        if (const auto it = mSessions.find(sessionId); it != mSessions.end()) {
            it->second->sendExecutionReport(report);
        } else {
            LOG_ERROR("Could not find session with ID {} to send execution report.",
                      sessionId);
        }
    }

    void FixServer::onMarketDataSnapshotFullRefresh(const MarketDataSnapshotFullRefresh& snapshot)
    {
        uint32_t sessionId = snapshot.targetSessionID;
        if (sessionId == 0) return;
        if (const auto it = mSessions.find(sessionId); it != mSessions.end()) {
            it->second->sendMarketDataSnapshotFullRefresh(snapshot);
        } else {
            LOG_ERROR("Could not find session with ID {} to send market data snapshot.",
                      sessionId);
        }
    }

    void FixServer::onMarketDataIncrementalRefresh(const MarketDataIncrementalRefresh& refresh)
    {
        uint32_t sessionId = refresh.targetSessionID;
        if (sessionId == 0) return;
        if (const auto it = mSessions.find(sessionId); it != mSessions.end()) {
            it->second->sendMarketDataIncrementalRefresh(refresh);
        } else {
            LOG_ERROR("Could not find session with ID {} to send market data incremental refresh.",
                      sessionId);
        }
    }

    void FixServer::doAccept()
    {
        if (!mAcceptor.is_open()) return;

        mAcceptor.async_accept(mSocket, [this](const std::error_code ec) {
            if (!ec) {
                const auto session = std::make_shared<FixSession>(
                        std::move(mSocket), *this, mTradingCore, mNextSessionId++);
                registerSession(session);
                session->start();
            }
            if (mAcceptor.is_open()) {
                doAccept();
            }
        });
    }

    void FixServer::registerSession(
            const std::shared_ptr<FixSession>& session)
    {
        mSessions[session->getSessionID()] = session;
    }

    void FixServer::unregisterSession(uint32_t sessionId)
    {
        mSessions.erase(sessionId);
        mTradingCore.unsubscribeFromMarketData(sessionId);
    }

} // namespace fix
