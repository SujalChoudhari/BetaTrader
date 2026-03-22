/**
 * @file FixClientSession.h
 * @brief Manages the FIX client-side TCP connection and protocol state machine.
 */

#pragma once

#include "fix_client/SeqNumStore.h"
#include "fix_client/FixMessageParser.h"
#include <asio.hpp>
#include <string>
#include <memory>
#include <functional>
#include <atomic>

namespace fix_client {

    /**
     * @enum SessionState
     * @brief The state machine status for the FIX client connection.
     */
    enum class FixClientState {
        Disconnected,
        Connecting,
        Connected,    // TCP up, Logon not sent
        LogonSent,
        Active,       // Logon ACK received
        LoggingOut
    };

    /**
     * @class FixClientSession
     * @brief High-performance asynchronous FIX client using ASIO.
     */
    class FixClientSession : public std::enable_shared_from_this<FixClientSession> {
    public:
        /**
         * @brief Callback type for when a fully parsed message falls out of the protocol engine.
         */
        using MessageCallback = std::function<void(const ParsedFixMessage&)>;

        /**
         * @brief Callback type for state transitions (e.g. Disconnected -> Active).
         */
        using StateChangeCallback = std::function<void(FixClientState)>;

        /**
         * @brief Constructs a new FIX Client Session.
         * @param ioContext The ASIO event loop to run on.
         * @param senderCompId This client's identification string (e.g. "MahaSahayak").
         * @param targetCompId The exchange's identification string (e.g. "BETA_EXCHANGE").
         */
        FixClientSession(asio::io_context& ioContext, 
                         const std::string& senderCompId,
                         const std::string& targetCompId);

        ~FixClientSession();

        /**
         * @brief Connects to the remote FIX server asynchronously.
         */
        void connect(const std::string& host, short port);

        /**
         * @brief Closes the socket and stops reading.
         */
        void disconnect();

        /**
         * @brief Sends a Logon (35=A) request. Must be called after connect() is successful.
         * @param heartbeatInterval Required heartbeat interval (default 30s).
         * @param forceReset If true, sends ResetSeqNumFlag=Y and drops seq store to 1.
         */
        void sendLogon(int heartbeatInterval = 30, bool forceReset = false);

        /**
         * @brief Sends a Logout (35=5) request.
         */
        void sendLogout(const std::string& reason = "");

        /**
         * @brief Queues a raw FIX message body to be properly headed, trailed, sequenced, and sent.
         * @param msgType The Tag 35 value (e.g. "D" for new order).
         * @param bodyStr The raw tag=value body string.
         */
        void sendMessage(const std::string& msgType, const std::string& bodyStr);

        /**
         * @brief Helper to send an authenticated UserRequest (35=BE)
         */
        void sendAuthRequest(const std::string& username, const std::string& password);

        // --- Configuration & Callbacks ---

        void setMessageCallback(MessageCallback cb) { mMessageCb = std::move(cb); }
        void setStateChangeCallback(StateChangeCallback cb) { mStateChangeCb = std::move(cb); }
        void setHeartbeatInterval(int seconds) { mHeartbeatInterval = seconds; }

        FixClientState getState() const { return mState; }
        const std::string& getSenderCompId() const { return mSenderCompId; }
        
    private:
        void doRead();
        void doWrite(std::shared_ptr<std::string> message);
        
        void handleProtocolMessage(const std::string& msgStr);
        void handleLogonResponse(const std::string& msgStr, uint32_t inSeq);
        void handleLogoutResponse(const std::string& msgStr);
        void handleHeartbeat(const std::string& msgStr);
        void handleTestRequest(const std::string& msgStr);
        void handleResendRequest(const std::string& msgStr);
        void handleSequenceReset(const std::string& msgStr);

        void changeState(FixClientState newState);
        void startHeartbeatTimer();
        void startTestRequestTimer();

        // Dependencies
        asio::io_context& mIoContext;
        asio::ip::tcp::socket mSocket;
        asio::ip::tcp::resolver mResolver;
        asio::steady_timer mHeartbeatTimer;
        asio::steady_timer mTestReqTimer;

        // Configuration
        std::string mSenderCompId;
        std::string mTargetCompId;
        int mHeartbeatInterval = 30;

        // State Machine
        std::atomic<FixClientState> mState{FixClientState::Disconnected};
        SeqNumStore mSeqStore;

        // Buffers
        static constexpr size_t ChunkSize = 8192;
        std::vector<char> mReadChunk;
        std::string mReadBuffer;

        // Callbacks
        MessageCallback mMessageCb;
        StateChangeCallback mStateChangeCb;
    };

} // namespace fix_client
