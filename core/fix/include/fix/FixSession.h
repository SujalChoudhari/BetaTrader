#pragma once

#include "fix/ExecutionReport.h"
#include "trading_core/TradingCore.h"
#include <asio.hpp>
#include <memory>
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

        asio::ip::tcp::socket mSocket; ///< The socket for this client connection.
        trading_core::TradingCore& mTradingCore; ///< Reference to the business logic layer.
        enum { MaxLength = 1024 }; ///< The maximum size of the read buffer.
        std::vector<char> mData; ///< The buffer for incoming socket data.
        uint32_t mSessionId; ///< The unique ID for this session.
    };

} // namespace fix
