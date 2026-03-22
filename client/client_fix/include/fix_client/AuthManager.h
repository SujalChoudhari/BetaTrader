/**
 * @file AuthManager.h
 * @brief Manages FIX 4.4 UserRequest based authentication.
 */

#pragma once

#include "fix_client/FixClientSession.h"
#include <string>
#include <memory>
#include <functional>

namespace fix_client {

    /**
     * @class AuthManager
     * @brief A layer above FixClientSession that handles the 35=BE/BF credentials exchange.
     * 
     * In BetaTrader, after the session is physically connected and Logged On (35=A),
     * a client must still authenticate its trading account using a UserRequest (35=BE)
     * before sending orders or subscribing to market data.
     */
    class AuthManager {
    public:
        using AuthCallback = std::function<void(bool success, const std::string& message)>;

        AuthManager(std::shared_ptr<FixClientSession> session);

        /**
         * @brief Sends the 35=BE message and awaits response.
         * @param username Account username
         * @param password Account password
         * @param callback Fired when 35=BF is received.
         */
        void authenticate(const std::string& username, const std::string& password, AuthCallback callback);

        /**
         * @brief Intercepts incoming messages looking for 35=BF
         * @return true if the message was handled by AuthManager, false if it should be passed to the application.
         */
        bool handleMessage(const ParsedFixMessage& parsed, const std::string& rawFix);

    private:
        std::shared_ptr<FixClientSession> mSession;
        AuthCallback mPendingCallback;
        std::string mPendingReqId;
    };

} // namespace fix_client
