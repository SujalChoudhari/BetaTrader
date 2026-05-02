#include "fix_client/AuthManager.h"
#include "common_fix/FixUtils.h"
#include "common_fix/Tags.h"
#include "logging/Logger.h"

#include <sstream>
#include <chrono>

namespace fix_client {

    AuthManager::AuthManager(std::shared_ptr<FixClientSession> session) 
        : mSession(session) {}

    void AuthManager::authenticate(const std::string& username, const std::string& password, AuthCallback callback) {
        if (!mSession || mSession->getState() != FixClientState::Active) {
            LOG_ERROR("AuthManager::authenticate - Session is not active.");
            if (callback) callback(false, "Session not active");
            return;
        }

        mPendingCallback = callback;
        if (!mNextReqId.empty()) {
            mPendingReqId = mNextReqId;
            mNextReqId = "";
        } else {
            mPendingReqId = "REQ_" + std::to_string(std::chrono::system_clock::now().time_since_epoch().count());
        }

        std::ostringstream body;
        body << "923=" << mPendingReqId << '\x01'      // UserRequestID
             << "924=1\x01"                             // UserRequestType = 1 (Logon)
             << "553=" << username << '\x01'            // Username
             << "554=" << password << '\x01';           // Password

        mSession->sendMessage("BE", body.str()); // 35=BE (UserRequest)
        LOG_INFO("AuthManager sent UserRequest (35=BE) for user '{}'", username);
    }

    bool AuthManager::handleMessage(const ParsedFixMessage& /*parsed*/, const std::string& rawFix) {
        // AuthManager needs to look for 35=BF (UserResponse)
        // Since FixMessageParser currently ignores it, we'll parse the raw string minimally here.
        
        auto map = fix::splitToMap(rawFix, '\x01');
        
        auto msgTypeIt = map.find(35);
        if (msgTypeIt == map.end() || msgTypeIt->second != "BF") {
            return false; // Not a UserResponse, pass to application
        }

        auto reqIdIt = map.find(923); // UserRequestID
        if (reqIdIt == map.end() || reqIdIt->second != mPendingReqId) {
            LOG_WARN("AuthManager received 35=BF with missing or unknown REQ_ID");
            return false;
        }

        auto statusIt = map.find(926); // UserStatus
        bool success = false;
        if (statusIt != map.end()) {
            // 1 = Logged In
            if (statusIt->second == "1") {
                success = true;
            }
        }

        std::string textMsg = "";
        auto textIt = map.find(927); // UserStatusText
        if (textIt != map.end()) {
            textMsg = std::string(textIt->second);
        }

        LOG_INFO("AuthManager received UserResponse (35=BF). Success: {}. Msg: {}", success, textMsg);

        if (mPendingCallback) {
            mPendingCallback(success, textMsg);
            mPendingCallback = nullptr;
            mPendingReqId = "";
        }

        return true; // We handled it, don't pass to UI
    }

} // namespace fix_client
