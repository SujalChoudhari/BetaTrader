#pragma once

#include "fix/ExecutionReport.h"
#include "fix/MarketDataSnapshotFullRefresh.h"
#include "fix/MarketDataIncrementalRefresh.h"
#include "fix/FixSession.h"
#include "trading_core/TradingCore.h"
#include <asio.hpp>
#include <map>
#include <memory>
#include <string> // For MDReqID mapping

namespace fix {

    /**
     * @class FixServer
     * @brief Manages all client FIX sessions and acts as the main network entry point.
     *
     * This class is responsible for accepting new TCP connections, creating a
     * `FixSession` for each one, and dispatching asynchronous events (like
     * execution reports) to the correct session. It is the top-level object
     * for the entire FIX gateway.
     */
    class FixServer {
    public:
        /**
         * @brief Constructs the FixServer.
         * @param ioContext The Asio I/O context that will run the server's event loop.
         * @param port The TCP port to listen on for new client connections.
         * @param tradingCore A reference to the application's trading core, used to submit orders.
         */
        FixServer(asio::io_context& ioContext, short port,
                  trading_core::TradingCore& tradingCore);

        /**
         * @brief Callback for receiving an execution report from the trading core.
         *
         * This method is called by the subscriber in `main`. It looks up the
         * target session using the ID in the report's `TargetCompID` field and
         * forwards the report for delivery.
         * @param report The execution report to be sent to a client.
         */
        void onExecutionReport(const ExecutionReport& report);

        /**
         * @brief Callback for receiving a market data snapshot full refresh from the trading core.
         * @param snapshot The market data snapshot to be sent to a client.
         */
        void onMarketDataSnapshotFullRefresh(const MarketDataSnapshotFullRefresh& snapshot);

        /**
         * @brief Callback for receiving a market data incremental refresh from the trading core.
         * @param refresh The market data incremental refresh to be sent to a client.
         */
        void onMarketDataIncrementalRefresh(const MarketDataIncrementalRefresh& refresh);

    private:
        /**
         * @brief Initiates an asynchronous accept operation to listen for a new client.
         *
         * When a connection is accepted, a new `FixSession` is created and this
         * method is called again to continue listening for more clients.
         */
        void doAccept();

        /**
         * @brief Registers a newly created session in the server's session map.
         * @param session A shared pointer to the new `FixSession`.
         */
        void registerSession(const std::shared_ptr<FixSession>& session);

        /**
         * @brief Removes a session from the map, typically after a disconnect.
         * @param sessionId The unique ID of the session to remove.
         */
        void unregisterSession(uint32_t sessionId);

        asio::ip::tcp::acceptor mAcceptor; ///< The Asio object that accepts incoming connections.
        asio::ip::tcp::socket mSocket;     ///< A socket for the next accepted connection.
        trading_core::TradingCore& mTradingCore; ///< A reference to the business logic layer.
        std::map<uint32_t, std::shared_ptr<FixSession>> mSessions; ///< A map of all active client sessions.
        uint32_t mNextSessionId = 1; ///< A simple counter to generate unique session IDs.

        /**
         * @brief Map to track which sessions are subscribed to which MDReqID for routing market data.
         *
         * The key is the Market Data Request ID (MDReqID), and the value is a vector
         * of weak pointers to FixSession objects that have subscribed to that MDReqID.
         */
        std::map<std::string, std::vector<std::weak_ptr<FixSession>>> mMarketDataSubscriptions;
    };

} // namespace fix
