#pragma once

#include "common_fix/ExecutionReport.h"
#include "common_fix/MarketDataIncrementalRefresh.h"
#include "common_fix/MarketDataSnapshotFullRefresh.h"
#include "trading_core/TradingCore.h"
#include <asio.hpp>
#include <memory>
#include <string>
#include <vector>

namespace fix {

    /**
     * @class FixSession
     * @brief Represents and manages a single connected client FIX session.
     *
     * Each instance of this class is responsible for the lifecycle of one client
     * connection. It handles reading incoming FIX messages, submitting them to the
     * trading core, and writing outgoing execution reports back to the client.
     * The session's lifetime is managed by a `std::shared_ptr`.
     */
    class FixSession : public std::enable_shared_from_this<FixSession> {
    public:
        /**
         * @brief Constructs a new FIX session.
         * @param socket The Asio TCP socket for this client connection.
         * @param tradingCore A reference to the application's trading core.
         * @param sessionId A unique identifier for this session.
         */
        FixSession(asio::ip::tcp::socket socket,
                   trading_core::TradingCore& tradingCore,
                   uint32_t sessionId);

        /**
         * @brief Starts the session's asynchronous read loop.
         */
        void start();

        /**
         * @brief Asynchronously sends an execution report to the client.
         *
         * This method serializes the `ExecutionReport` into a binary FIX message
         * and writes it to the socket.
         * @param report The report to send.
         */
        void sendExecutionReport(const ExecutionReport& report);

        /**
         * @brief Asynchronously sends a market data snapshot full refresh to the client.
         * @param snapshot The market data snapshot to send.
         */
        void sendMarketDataSnapshotFullRefresh(const MarketDataSnapshotFullRefresh& snapshot);

        /**
         * @brief Asynchronously sends a market data incremental refresh to the client.
         * @param refresh The market data incremental refresh to send.
         */
        void sendMarketDataIncrementalRefresh(const MarketDataIncrementalRefresh& refresh);

        /**
         * @brief Gets the unique identifier for this session.
         * @return The session ID.
         */
        uint32_t getSessionID() const;

    private:
        /**
         * @brief Initiates an asynchronous read operation on the socket.
         *
         * This is the core of the session's event loop. When data is received,
         * it is processed, and another read is initiated.
         */
        void doRead();

        /**
         * @brief Handles a parsed FIX message.
         * @param fixMessage The raw FIX message string.
         * @param msgType The MsgType of the FIX message.
         * //TODO: Refine handleFixMessage to dispatch to specific handlers based on MsgType more robustly.
         */
        void handleFixMessage(const std::string& fixMessage, char msgType);

        /**
         * @brief Handles an incoming Cancel Order Request.
         * @param fixMessage The raw FIX message string for the cancel order.
         */
        void handleCancelOrderRequest(const std::string& fixMessage);

        /**
         * @brief Handles an incoming Modify Order Request.
         * @param fixMessage The raw FIX message string for the modify order.
         */
        void handleModifyOrderRequest(const std::string& fixMessage);

        /**
         * @brief Handles an incoming Market Data Request.
         * @param fixMessage The raw FIX message string for the market data request.
         */
        void handleMarketDataRequest(const std::string& fixMessage);

        /**
         * @brief Initiates an asynchronous write operation to send a message to the client.
         * @param message The message string to be sent.
         */
        void doWrite(const std::string& message);

        asio::ip::tcp::socket mSocket; ///< The socket for this client connection.
        trading_core::TradingCore& mTradingCore; ///< Reference to the business logic layer.
        enum { MaxLength = 1024 }; ///< The maximum size of the read buffer.
        std::vector<char> mData; ///< The buffer for incoming socket data.
        uint32_t mSessionId; ///< The unique ID for this session.
        std::string mReadBuffer; ///< Buffer to accumulate partial FIX messages.
    };

} // namespace fix
