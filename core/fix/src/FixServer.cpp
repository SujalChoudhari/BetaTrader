#include "fix/FixServer.h"
#include "fix/FixSession.h"
#include "logging/Logger.h"

namespace fix {

    FixServer::FixServer(asio::io_context& ioContext, short port,
                         trading_core::TradingCore& tradingCore)
        : mAcceptor(ioContext,
                    asio::ip::tcp::endpoint(asio::ip::tcp::v4(), port)),
          mSocket(ioContext), mTradingCore(tradingCore)
    {
        doAccept();
    }

    void FixServer::onExecutionReport(const ExecutionReport& report)
    {
        uint32_t sessionId = report.getTargetCompId();
        if (const auto it = mSessions.find(sessionId); it != mSessions.end()) {
            it->second->sendExecutionReport(report);
        } else {
            LOG_ERROR("Could not find session with ID {} to send execution report.",
                      sessionId);
        }
    }

    void FixServer::doAccept()
    {
        mAcceptor.async_accept(mSocket, [this](const std::error_code ec) {
            if (!ec) {
                const auto session = std::make_shared<FixSession>(
                        std::move(mSocket), mTradingCore, mNextSessionId++);
                registerSession(session);
                session->start();
            }
            doAccept();
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
    }

} // namespace fix
